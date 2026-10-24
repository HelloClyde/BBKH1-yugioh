# 发布约定

公开仓库：[HelloClyde/BBKH1-yugioh](https://github.com/HelloClyde/BBKH1-yugioh)。初始版本 `0.1.0`，标签 `v0.1.0`；实际构建与发布状态分别以仓库 Actions 和 Releases 页面为准。

## 从干净检出构建

克隆获准发布的应用仓库后，执行 README 中的子模块初始化、依赖安装、工具链安装与 `python tools/ci.py --cc <主机GCC路径>`。不要复制开发工作区的生成 BDA 当作构建结果。SDK 必须固定、干净，MIPS GCC 版本必须为 15.2.0。

本地 CI 入口在两个输出目录分别编译，对照 BDA 和 ELF 的 SHA-256；检查源文件包含闭包、真实截图链接、SDK gitlink 和 Git 跟踪内容；运行主机完整规则/存档测试与 MIPS 存档测试；最后检查 BDA 包装和 ZIP 内程序字节。生成 `build/ci-verification.json` 和 `dist/SHA256SUMS.txt`。

## 远程发布

确定远程仓库所有者、名称、可见性和发布授权后再创建仓库并推送。先检查同名仓库、tag、Release，避免覆盖已有项目。

1. 推送经过本地验证的 main 提交，等待 main 构建通过。
2. 核对 `VERSION`，创建指向同一提交的 `v<版本>` 标签并推送。
3. 等待标签构建。release job 使用 build job 同一次构建上传的 artifact；不重新编译。
4. 下载 artifact 后按每个文件的 SHA-256 检查；发布时使用该次的 BDA、ZIP、manifest、说明与校验清单。
5. 检查 Release 为 prerelease，逐个下载并复核哈希，记录 tag 提交、Actions 和 Release URL。

CI 默认只有 contents:read；发布 job 单独 contents:write。main/PR/manual 只生成临时 artifacts。发布正文取自 UTF-8 `RELEASE_NOTES.md`。脚本不使用 `--clobber`；已有同名 Release 时停止，先确认它是否为同一次发布。

自动发布资产为 `JOEY.bda`、`BBKH1-yugioh-v<版本>-code-only.zip`、`build-manifest.json`、`RELEASE_NOTES.md`、`SHA256SUMS.txt`。`v0.1.1` 另附资源 ZIP 和 `RESOURCES-SHA256SUMS.txt`，ZIP 只含五个兼容资源，目录为 `应用/数据/yugioh/`；资源不加入 Git。不上传 SDK 工具链、固件、原 PC 程序或存档。

Actions 依赖在 2026-10-24 对照官方仓库确认接口并固定到提交。构建 job 可在没有商业资源/私人固件的环境运行；不能据此声称完整固件或真机验证通过。
