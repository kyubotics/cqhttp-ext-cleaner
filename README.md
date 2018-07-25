# CoolQ HTTP API 插件 - 垃圾清理扩展

[![License](https://img.shields.io/badge/license-MIT%20License-blue.svg)](LICENSE)
[![Release](https://img.shields.io/github/release/richardchien/cqhttp-ext-cleaner.svg)](https://github.com/richardchien/cqhttp-ext-cleaner/releases)

通过扩展（Extension）给 CoolQ HTTP API 插件提供后台自动清理垃圾功能。

## 使用方式

将 [Release](https://github.com/richardchien/cqhttp-ext-cleaner/releases) 中下载的 `Cleaner.dll` 放到酷 Q 的 `app\io.github.richardchien.coolqhttpapi\extensions` 目录下（没有则创建），然后配置文件中添加 `cleaner.enable = true`，重启插件即可。

## 配置

需要对扩展进行配置，它才会实际工作，默认情况下什么都不做。

| 配置项 | 默认值 | 说明 |
| ----- | ----- | --- |
| `cleaner.enable` | `false` | 是否启用垃圾清理功能 |
| `cleaner.supervise_interval` | `3600000`（1 小时） | 监控时间周期，单位毫秒 |
| `cleaner.coolq_log_max_size` | `0` | 最大酷 Q 日志文件大小，0 表示不清理 |
| `cleaner.data_files_max_count.bface` | `0` | 最大 bface 数据文件数量，0 表示不清理 |
| `cleaner.data_files_max_count.image` | `0` | 最大 image 数据文件数量，0 表示不清理 |
| `cleaner.data_files_max_count.record` | `0` | 最大 record 数据文件数量，0 表示不清理 |
| `cleaner.data_files_max_count.show` | `0` | 最大 show 数据文件数量，0 表示不清理 |
