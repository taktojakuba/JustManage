#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <fstream>
#include <string>
#include <unistd.h>

static std::string run(const std::string &cmd) {
  char buf[64] = {};
  FILE *p = popen(cmd.c_str(), "r");
  fgets(buf, sizeof buf, p);
  pclose(p);
  return buf;
}

static int do_editor() {
  FILE *p = popen("printf 'VSC\\nNvim\\nOpenProject-VSC\\nOpenProject-NVIM' | "
                  "rofi -dmenu -p Editor",
                  "r");
  char buf[32];
  if (!fgets(buf, sizeof buf, p))
    return 1;
  std::string c = buf;
  c.pop_back();

  if (c == "VSC")
    system("code");
  else if (c == "Nvim")
    system("foot -e nvim");
  return 0;
}

static int do_fscreen(std::string output = "HDMI-A-1") {
  char ts[32];
  time_t t = time(nullptr);
  strftime(ts, sizeof ts, "%F_%H-%M-%S", localtime(&t));
  std::string f =
      std::string(getenv("HOME")) + "/Pictures/Screenshots/" + ts + ".png";

  std::string cmd = "grim -o \"" + output + "\" \"" + f + "\"";
  if (system(cmd.c_str()))
    return 1;

  system(("notify-send -u low 'Screenshot saved' \"" + f + "\"").c_str());
  system(("wl-copy < \"" + f + "\"").c_str());
  return 0;
}
static int do_autostart() {
  system("justbar >/dev/null 2>&1 &");
  system("mako >/dev/null 2>&1 &");
  system("wbg ~/Pictures/Wallpapers/monoforest.png >/dev/null 2>&1 &");
  system("foot --server >/dev/null 2>&1 &");
  system("wlr-randr --output HDMI-A-1 --mode 1920x1080@119.878998Hz >/dev/null "
         "2>&1 &");
  system("wl-clip-persist --clipboard regular --reconnect-tries 0 >/dev/null "
         "2>&1 &");
  system("wl-paste --type text --watch cliphist store >/dev/null 2>&1 &");
  return 0;
}
static int do_launch() {
  setenv("XCURSOR_THEME", "Future-dark Cursors", 1);
  setenv("XCURSOR_SIZE", "24", 1);
  setenv("XDG_CURRENT_DESKTOP", "weebwl", 1);
  setenv("XDG_SESSION_TYPE", "wayland", 1);
  setenv("DBUS_SESSION_BUS_ADDRESS",
         ("unix:path=/run/user/" + std::to_string(getuid()) + "/bus").c_str(),
         1);
  setenv("WLR_RENDERER", "gles2", 1);
  execlp("jr","jr", "weebwl", "-s", "jm system autostart",nullptr);
  return 127;
}

static int do_power() {
  FILE *p = popen(
      "printf 'Shutdown\\nReboot\\nLogout\\nSuspend' | rofi -dmenu -p Power",
      "r");
  char buf[32];
  if (!fgets(buf, sizeof buf, p))
    return 1;
  std::string c = buf;
  c.pop_back();

  if (c == "Shutdown")
    system("shutdown now");
  else if (c == "Reboot")
    system("reboot");
  else if (c == "Logout")
    system("loginctl terminate-user $USER");
  else if (c == "Suspend")
    system("loginctl suspend");
  return 0;
}

static int do_powerprofile() {
  FILE *p = popen("printf 'power-saver\\nbalanced\\nperformance' | rofi -dmenu "
                  "-p Power-Profile",
                  "r");
  char buf[32];
  if (!fgets(buf, sizeof buf, p))
    return 1;
  std::string c = buf;
  c.pop_back();

  if (c == "power-saver")
    system("powerprofilesctl set power-saver");
  else if (c == "balanced")
    system("powerprofilesctl set balanced");
  else if (c == "performance")
    system("powerprofilesctl set performance");
  return 0;
}

static int do_sscreen() {
  char ts[32];
  time_t t = time(nullptr);
  strftime(ts, sizeof ts, "%F_%H-%M-%S", localtime(&t));
  std::string f =
      std::string(getenv("HOME")) + "/Pictures/Screenshots/" + ts + ".png";

  if (system(("grim -g \"$(slurp)\" \"" + f + "\"").c_str()))
    return 1;
  system(("notify-send -u low 'Screenshot saved' \"" + f + "\"").c_str());
  system(("wl-copy < \"" + f + "\"").c_str());
  return 0;
}
static int do_vol(char action = '\0') {
  const char *id_file = "/tmp/volume-notify-id";

  if (action == 'd') {
    system("wpctl set-volume @DEFAULT_AUDIO_SINK@ 5%-");
  } else if (action == 'u') {
    system("wpctl set-volume @DEFAULT_AUDIO_SINK@ 5%+");
  } else if (action == '\0') {
    system("wpctl set-mute @DEFAULT_AUDIO_SINK@ toggle");
  } else {
    return -1;
  }

  std::string status = run("wpctl get-volume @DEFAULT_AUDIO_SINK@");
  std::string msg;

  if (status.find("MUTED") != std::string::npos) {
    msg = "Muted";
  } else {
    double v = 0;
    sscanf(status.c_str(), "Volume: %lf", &v);
    msg = "Volume " + std::to_string((int)(v * 100)) + "%";
  }

  std::string id;
  std::ifstream(id_file) >> id;

  std::string cmd = "notify-send -u low -p ";
  if (!id.empty())
    cmd += "-r " + id + " ";
  cmd += "'Audio " + msg + "'";

  std::string notification_id = run(cmd);
  std::ofstream(id_file) << notification_id;

  return 0;
}
static int do_wallpaper() {
  std::string dir = std::string(getenv("HOME")) + "/Pictures/Wallpapers";
  FILE *p = popen(("find '" + dir +
                   "' -maxdepth 1 -type f \\( -name '*.jpg' -o -name '*.jpeg' "
                   "-o -name '*.png' -o -name '*.webp' \\) -printf '%f\\n' | "
                   "sort | rofi -dmenu -p Wallpaper")
                      .c_str(),
                  "r");
  char buf[256];
  if (!fgets(buf, sizeof buf, p))
    return 1;
  std::string w = buf;
  w.pop_back();

  system("pkill wbg 2>/dev/null");
  system(("wbg -s '" + dir + "/" + w + "' >/dev/null 2>&1 &").c_str());
  return 0;
}

int main(int argc, char **argv) {
  std::string action;
  std::string argument;
  std::string extrarg;
  if (argc > 1) {
    action = argv[1];
    if (argc > 2) {
      argument = argv[2];
    }
    if (argc > 3) {
      extrarg = argv[3];
    }
  } else {
    return 1;
  }
  if (action == "vol") {
    if (argument == "up")
      return do_vol('u');
    if (argument == "down")
      return do_vol('d');
    if (argument == "mute")
      return do_vol();
    else
      return do_vol();
  }
  if (action == "screen") {
    if (argument == "full")
      return do_fscreen(extrarg);
    if (argument == "select")
      return do_sscreen();
  }
  if (action == "menu") {
    if (argument == "editor")
      return do_editor();
    if (argument == "wallpaper")
      return do_wallpaper();
    if (argument == "power")
      return do_power();
    if (argument == "powerprofile")
      return do_powerprofile();
  }
  if (action == "system") {
    if (argument == "launch")
      return do_launch();
    if (argument == "autostart")
      return do_autostart();
  }
  return 1;
}
