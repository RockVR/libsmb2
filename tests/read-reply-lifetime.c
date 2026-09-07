/* PR quick regression: exercise the production high-level read callback. */
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
static void *watched;
static int released;
static void tracked_free(void *p) {
        if (p && p == watched) released++;
        free(p);
}
#define free tracked_free
#include "../lib/libsmb2.c"
#undef free

static int expected_count, calls;
static void done(struct smb2_context *ctx, int status, void *data, void *private_data) {
        struct smb2_read_cb_data *read = data;
        assert(status == expected_count);
        if (expected_count > 0) {
                assert(released == 1); /* fail before the fix */
                assert(read->buf[0] == 42 && read->buf[15] == 42);
        } else {
                assert(released == 0);
        }
        calls++;
}
static void check(struct smb2_context *ctx, int status, uint32_t length) {
        struct read_data *rd = calloc(1, sizeof(*rd));
        struct smb2fh fh = {0};
        struct smb2_read_reply reply = {0};
        unsigned char buffer[16];
        memset(buffer, 42, sizeof(buffer));
        rd->cb = done;
        rd->read_cb_data.fh = &fh;
        rd->read_cb_data.buf = buffer;
        rd->read_cb_data.offset = 100;
        reply.data_length = length;
        /* Empty replies do not initialize data in the parser. */
        reply.data = length ? malloc(length) : (void *)1;
        watched = length ? reply.data : NULL;
        released = calls = 0;
        expected_count = status == SMB2_STATUS_ACCESS_DENIED ? -EACCES : (int)length;
        read_cb(ctx, status, &reply, rd);
        assert(calls == 1);
        if (status == SMB2_STATUS_SUCCESS) assert(fh.offset == 100 + length);
        if (length) assert(released == 1);
        watched = NULL;
}
int main(void) {
        struct smb2_context *ctx = smb2_init_context();
        assert(ctx);
        check(ctx, SMB2_STATUS_SUCCESS, 16);
        check(ctx, SMB2_STATUS_SUCCESS, 0);
        check(ctx, SMB2_STATUS_END_OF_FILE, 0);
        check(ctx, SMB2_STATUS_ACCESS_DENIED, 0);
        smb2_destroy_context(ctx);
        puts("read reply lifetime: nonempty / empty / EOF / error PASS");
}
