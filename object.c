// object.c — Content-addressable object store

#include "pes.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <openssl/evp.h>

// ─── PROVIDED ────────────────────────────────────────────────────────────────

void hash_to_hex(const ObjectID *id, char *hex_out) {
    for (int i = 0; i < HASH_SIZE; i++) {
        sprintf(hex_out + i * 2, "%02x", id->hash[i]);
    }
    hex_out[HASH_HEX_SIZE] = '\0';
}

int hex_to_hash(const char *hex, ObjectID *id_out) {
    if (strlen(hex) < HASH_HEX_SIZE) return -1;
    for (int i = 0; i < HASH_SIZE; i++) {
        unsigned int byte;
        if (sscanf(hex + i * 2, "%2x", &byte) != 1) return -1;
        id_out->hash[i] = (uint8_t)byte;
    }
    return 0;
}

void compute_hash(const void *data, size_t len, ObjectID *id_out) {
    unsigned int hash_len;
    EVP_MD_CTX *ctx = EVP_MD_CTX_new();
    EVP_DigestInit_ex(ctx, EVP_sha256(), NULL);
    EVP_DigestUpdate(ctx, data, len);
    EVP_DigestFinal_ex(ctx, id_out->hash, &hash_len);
    EVP_MD_CTX_free(ctx);
}

void object_path(const ObjectID *id, char *path_out, size_t path_size) {
    char hex[HASH_HEX_SIZE + 1];
    hash_to_hex(id, hex);
    snprintf(path_out, path_size, "%s/%.2s/%s", OBJECTS_DIR, hex, hex + 2);
}

int object_exists(const ObjectID *id) {
    char path[512];
    object_path(id, path, sizeof(path));
    return access(path, F_OK) == 0;
}

// ─── IMPLEMENTED ─────────────────────────────────────────────────────────────

int object_write(ObjectType type, const void *data, size_t len, ObjectID *id_out) {
    char type_str[10];

    if (type == OBJ_BLOB) strcpy(type_str, "blob");
    else if (type == OBJ_TREE) strcpy(type_str, "tree");
    else if (type == OBJ_COMMIT) strcpy(type_str, "commit");
    else return -1;

    char header[64];
    int header_len = snprintf(header, sizeof(header), "%s %zu", type_str, len) + 1;

    size_t total_len = header_len + len;
    char *buffer = malloc(total_len);
    if (!buffer) return -1;

    memcpy(buffer, header, header_len);
    memcpy(buffer + header_len, data, len);

    compute_hash(buffer, total_len, id_out);

    if (object_exists(id_out)) {
        free(buffer);
        return 0;
    }

    char hex[HASH_HEX_SIZE + 1];
    hash_to_hex(id_out, hex);

    char dir_path[512];
    snprintf(dir_path, sizeof(dir_path), "%s/%.2s", OBJECTS_DIR, hex);

    mkdir(".pes", 0755);
    mkdir(OBJECTS_DIR, 0755);
    mkdir(dir_path, 0755);

    char final_path[512];
    object_path(id_out, final_path, sizeof(final_path));

    char temp_path[512];
    snprintf(temp_path, sizeof(temp_path), "%s/tmpXXXXXX", dir_path);

    int fd = mkstemp(temp_path);
    if (fd < 0) {
        free(buffer);
        return -1;
    }

    if (write(fd, buffer, total_len) != (ssize_t)total_len) {
        close(fd);
        unlink(temp_path);
        free(buffer);
        return -1;
    }

    fsync(fd);
    close(fd);

    if (rename(temp_path, final_path) != 0) {
        unlink(temp_path);
        free(buffer);
        return -1;
    }

    int dir_fd = open(dir_path, O_DIRECTORY);
    if (dir_fd >= 0) {
        fsync(dir_fd);
        close(dir_fd);
    }

    free(buffer);
    return 0;
}


int object_read(const ObjectID *id, ObjectType *type_out, void **data_out, size_t *len_out) {
    char path[512];
    object_path(id, path, sizeof(path));

    FILE *fp = fopen(path, "rb");
    if (!fp) return -1;

    fseek(fp, 0, SEEK_END);
    size_t size = ftell(fp);
    rewind(fp);

    char *buffer = malloc(size);
    if (!buffer) {
        fclose(fp);
        return -1;
    }

    if (fread(buffer, 1, size, fp) != size) {
        fclose(fp);
        free(buffer);
        return -1;
    }
    fclose(fp);

    ObjectID computed;
    compute_hash(buffer, size, &computed);

    if (memcmp(computed.hash, id->hash, HASH_SIZE) != 0) {
        free(buffer);
        return -1;
    }

    char *data_start = memchr(buffer, '\0', size);
    if (!data_start) {
        free(buffer);
        return -1;
    }

    if (strncmp(buffer, "blob", 4) == 0)
        *type_out = OBJ_BLOB;
    else if (strncmp(buffer, "tree", 4) == 0)
        *type_out = OBJ_TREE;
    else if (strncmp(buffer, "commit", 6) == 0)
        *type_out = OBJ_COMMIT;
    else {
        free(buffer);
        return -1;
    }

    data_start++;
    size_t data_len = size - (data_start - buffer);

    void *out = malloc(data_len);
    if (!out) {
        free(buffer);
        return -1;
    }

    memcpy(out, data_start, data_len);

    *data_out = out;
    *len_out = data_len;

    free(buffer);
    return 0;
}
