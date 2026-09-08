/* PR quick regression against production socket/init code. */
#include <assert.h>
#include "../lib/socket.c"
static int frees;
static void count_free(void *p) { ++frees; free(p); }
int main(void) {
    struct smb2_context *smb = smb2_init_context();
    assert(smb);
    struct smb2_io_vectors vectors = {0};
    uint8_t byte = 42;
    for (int i = 0; i < SMB2_MAX_VECTORS; ++i)
        assert(smb2_add_iovector(smb, &vectors, &byte, 1, NULL));
    assert(!smb2_add_iovector(smb, &vectors, malloc(4), 4, count_free));
    assert(frees == 1 && vectors.niov == SMB2_MAX_VECTORS);
    smb2_free_iovector(smb, &vectors);
    int pairs[3][2];
    smb->connecting_fds = malloc(3 * sizeof(t_socket));
    smb->connecting_fds_count = 3;
    for (int i = 0; i < 3; ++i) {
        assert(socketpair(AF_UNIX, SOCK_STREAM, 0, pairs[i]) == 0);
        smb->connecting_fds[i] = pairs[i][0];
    }
    smb2_close_connecting_fd(smb, pairs[0][0]);
    assert(smb->connecting_fds_count == 2);
    assert(smb->connecting_fds[0] == pairs[1][0]);
    assert(smb->connecting_fds[1] == pairs[2][0]);
    for (int i = 0; i < 3; ++i) close(pairs[i][1]);
    smb2_destroy_context(smb);
    puts("iovec boundary/one-owner free/connecting fd removal PASS");
}
