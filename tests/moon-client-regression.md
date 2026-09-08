# Moon Player 客户端升级回归

状态：候选升级，2026-09-07；来源：用户要求从 main 升级 AMSMB2/libsmb2。

基线为上游 `38b5edf`（包含 4.0.3 捆绑版本后的 iovec 边界、分配失败检查和目录通知解码修复），补入 `1042068` 的 connecting_fds memmove 字节数修复。

不纳入更晚的文件句柄所有权/取消 API 重构。`39d9596` 已评估但不采用：本基线 smb2_add_iovector 失败时自己调用 free_cb，调用方再 free 会重复释放。`95cc544` 的 next_pdu 清理与连接资源清理留待完整上下文升级评估。本次不声称已覆盖 master 的全部修复。

本地修复：LOGOFF 回调关闭 socket 后，读取循环立即正常返回，避免再次 readv(-1) 将正常断开报为错误。实际 loopback 回归在修复前断开报 errno 9，修复后通过。

PR 快速入口：`tests/test-client-lifecycle.sh`。编译生产 socket.c/init.c，验证 iovec 上界、失败时缓冲区仅释放一次、多连接描述符删除保持剩余顺序。
发布前入口：AMSMB2 的 `Scripts/test-read-memory.sh`，验证真实 SMB 读取/随机访问/EOF/错误恢复/断开内存；App 的 `Scripts/test-smb.sh release` 聚合并构建 visionOS Simulator。

旧高层 read_cb free(rep->data) 补丁不适用于此版本，不得叠加；使用上游 PDU 所有权管理。
