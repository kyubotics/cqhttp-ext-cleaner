#include <cqhttp/extension/extension.h>

#include <chrono>
#include <condition_variable>
#include <filesystem>
#include <thread>

using namespace std;
using namespace cqhttp::extension;
using nlohmann::json;
namespace fs = filesystem;

static wstring s2ws(const string &s) { return wstring_convert<codecvt_utf8<wchar_t>, wchar_t>().from_bytes(s); }

struct Cleaner : Extension {
    Info info() const override {
        static Info i;
        i.name = u8"垃圾清理";
        i.version = "0.0.2";
        i.build_number = 2;
        i.description = u8"提供后台自动清理垃圾功能。";
        return i;
    }

#define CONF_PREFIX "cleaner."

    void on_create(Context &ctx) override {
        enable_ = ctx.get_config_bool(CONF_PREFIX "enable", false);
        if (enable_) {
            supervise_interval_ = chrono::milliseconds(
                ctx.get_config_integer(CONF_PREFIX "supervise_interval", 1 * 60 * 60 * 1000 /* 1 hour */));

            const auto result = ctx.call_action("get_login_info");
            if (result.code == ActionResult::Codes::OK) {
                self_id_str_ = to_string(result.data.at("user_id").get<int64_t>());
            } else {
                logger.warning(u8"获取机器人自身 QQ 失败，部分功能可能无法使用");
            }

            coolq_log_max_size_ = ctx.get_config_integer(CONF_PREFIX "coolq_log_max_size", 0);
            for (const auto data_dir : initializer_list<string>{"image", "record", "show", "bface"}) {
                data_files_max_count_[data_dir] =
                    ctx.get_config_integer(CONF_PREFIX "data_files_max_count." + data_dir, 0);
            }

            // this is safe, because call_action() do not keep any reference to ctx
            call_action_ = ctx.__bridge.call_action;

            supervisor_thread_ = thread([&] {
                {
                    unique_lock<mutex> lock(m_);
                    supervising_ = true;
                }
                for (;;) {
                    supervise();
                    if (unique_lock<mutex> lock(m_);
                        cv_.wait_for(lock, supervise_interval_, [&] { return !supervising_; })) {
                        // wait until we should stop supervising
                        break;
                    }
                }
            });
        }
    }

    void on_destroy(Context &ctx) override {
        {
            unique_lock<mutex> lock(m_);
            supervising_ = false;
        }
        cv_.notify_all();
        if (supervisor_thread_.joinable()) {
            supervisor_thread_.join();
        }
    }

    void supervise() const {
        const auto result = call_action_("get_version_info", {});
        if (result.code != ActionResult::Codes::OK) {
            logger.error(u8"获取酷 Q 根目录路径失败，无法进行垃圾清理");
            return;
        }

        const auto coolq_root = result.data.at("coolq_directory").get<string>();

        for (auto it = data_files_max_count_.begin(); it != data_files_max_count_.end(); ++it) {
            const auto data_dir = it.key();
            const auto data_dir_full_path = coolq_root + "data\\" + data_dir;
            const auto max_file_count = it.value().get<size_t>();

            if (max_file_count == 0) {
                continue;
            }

            logger.debug(u8"正在检查 " + data_dir + " 目录……");

            auto count = 0;
            auto should_clean = false;
            for (const auto &_ : fs::directory_iterator(s2ws(data_dir_full_path))) {
                count++;
                if (count > max_file_count) {
                    should_clean = true;
                    break;
                }
            }

            if (should_clean) {
                logger.info(u8"正在清理 " + data_dir + " 目录……");
                call_action_("clean_data_dir", {{"data_dir", data_dir}});
            }
        }

        if (!self_id_str_.empty() && coolq_log_max_size_ != 0) {
            const auto coolq_log_file = coolq_root + "data\\" + self_id_str_ + "\\logv1.db";
            try {
                logger.debug(u8"正在检查酷 Q 日志文件……");
                if (fs::file_size(s2ws(coolq_log_file)) > coolq_log_max_size_) {
                    logger.info(u8"正在清理酷 Q 日志文件，此操作需要重启酷 Q，即将重启……");
                    call_action_("set_restart", {{"clean_log", true}});
                }
            } catch (fs::filesystem_error &) {
            }
        }
    }

private:
    bool enable_{};
    chrono::milliseconds supervise_interval_{};
    thread supervisor_thread_;
    bool supervising_ = false;
    mutex m_;
    condition_variable cv_;

    string self_id_str_;
    size_t coolq_log_max_size_{};
    json data_files_max_count_;

    function<ActionResult(const string &action, const json &params)> call_action_;
};

EXTENSION_CREATOR { return make_shared<Cleaner>(); }
