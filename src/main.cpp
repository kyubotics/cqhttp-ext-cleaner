#include <cqhttp/extension/extension.h>

using namespace std;
using namespace cqhttp::extension;
using nlohmann::json;

struct Cleaner : Extension {
    Info info() const override {
        static Info i;
        i.name = "垃圾清理";
        i.version = "0.0.1";
        i.build_number = 1;
        i.description = "提供后台自动清理垃圾功能。";
        return i;
    }

#define CONF_PREFIX "cleaner."

    void on_create(Context &ctx) override {
        enable_ = ctx.get_config_bool(CONF_PREFIX "enable", false);
        if (enable_) {
        }
    }

    void on_destroy(Context &ctx) override {}

private:
    bool enable_{};
};

EXTENSION_CREATOR { return make_shared<Cleaner>(); }
