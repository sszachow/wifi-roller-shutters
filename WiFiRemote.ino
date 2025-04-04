#include <WiFi.h>
#include "WiFiConsts.h"
#include <NetworkClient.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <LittleFS.h>

#define FORMAT_LITTLEFS_IF_FAILED true

#define RIGHT_BUTTON 23
#define MENU_BUTTON 22
#define UP_BUTTON 21
#define DOWN_BUTTON 19

#define PRESS_DOWN_MS 100
#define PRESS_UP_MS 150

#define SCREEN_TIMEOUT_S 60
#define SCREEN_WAKEUP_MS 1500

#define MAX_CHANNEL 16

WebServer server(80);

uint8_t current_channel = 1;
uint8_t max_channel = MAX_CHANNEL;

uint32_t stored_millis = 0;

enum direction {
  UP,
  DOWN
};

void setup_buttons() {
  pinMode(RIGHT_BUTTON, OUTPUT);
  pinMode(MENU_BUTTON, OUTPUT);
  pinMode(UP_BUTTON, OUTPUT);
  pinMode(DOWN_BUTTON, OUTPUT);

  digitalWrite(RIGHT_BUTTON, HIGH);
  digitalWrite(MENU_BUTTON, HIGH);
  digitalWrite(UP_BUTTON, HIGH);
  digitalWrite(DOWN_BUTTON, HIGH);
}

void setup_serial() {
  Serial.begin(115200);
}

void setup_wifi() {
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
}

void setup_mdns() {
  if (!MDNS.begin("blinds")) {
    while (1) {
      delay(1000);
    }
  }
  MDNS.addService("http", "tcp", 80);
}

void setup_fs() {
  while (!LittleFS.begin(FORMAT_LITTLEFS_IF_FAILED)) {
    delay(1000);
  }
}

uint8_t calculate_taps(uint8_t channel) {
  uint8_t num_of_taps = 0;
  if (channel > current_channel) {
    num_of_taps = channel - current_channel;
  }
  if (channel < current_channel) {
    num_of_taps = max_channel - (current_channel - channel);
  }
  return num_of_taps;
}

void set_channel(uint8_t channel) {
  uint8_t num_of_taps = calculate_taps(channel);

  press_right_button(num_of_taps);
  current_channel = channel;
}

void send_remote_command(direction dir, uint8_t channel) {
  wake_up_screen();
  set_channel(channel);
  if (dir == UP) {
    press_up_button();
  } else {
    press_down_button();
  }
  update_stored_millis();
}

void handle_command(direction dir) {
  if (server.args() == 0 || !server.hasArg("channel")) {
    server.send(404, "text/plain", "Not supported");
    return;
  }

  uint8_t channel = server.arg("channel").toInt();

  if (channel < 1 || channel > MAX_CHANNEL) {
    server.send(404, "text/plain", "Not supported");
    return;
  }

  send_remote_command(dir, channel);
  server.send(200);
}

void handle_file(String path, String type) {
  File f = LittleFS.open(path);

  if (!f.available()) {
    server.send(404, "text/plain", "File not found");
    return;
  }
  server.send(200, type, f.readString());
  f.close();
}

void handle_get_channel() {
  server.send(200, "text/html", String(current_channel));
}

void setup() {
  // put your setup code here, to run once:
  setup_buttons();
  setup_serial();
  setup_wifi();
  setup_mdns();
  setup_fs();

  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  server.on("/", HTTP_GET, []() {
    handle_file("/index.html", "text/html");
  });

  server.on("/style.css", HTTP_GET, []() {
    handle_file("/style.css", "text/css");
  });

  server.on("/channel_names.js", HTTP_GET, []() {
    handle_file("/channel_names.js", "text/javascript");
  });

  server.on("/up", HTTP_GET, []() {
    handle_command(UP);
  });

  server.on("/down", HTTP_GET, []() {
    handle_command(DOWN);
  });

  server.on("/get_channel", HTTP_GET, handle_get_channel);

  server.onNotFound([]() {
    server.send(404, "text/plain", "Not found");
  });

  server.begin();
}

void my_delay(uint32_t ms) {
  uint32_t last_millis = millis();
  uint32_t current_millis = 0;

  while (true) {
    current_millis = millis();
    if (current_millis - last_millis >= ms) {
      return;
    }
  }
}

void press_button(uint8_t button) {
  digitalWrite(button, LOW);
  my_delay(PRESS_DOWN_MS);
  digitalWrite(button, HIGH);
  my_delay(PRESS_UP_MS);
}

void press_right_button() {
  press_button(RIGHT_BUTTON);
}

void press_right_button(uint8_t times) {
  for (int i = 0; i < times; i++) {
    press_right_button();
  }
}

void press_menu_button() {
  press_button(MENU_BUTTON);
}

void press_up_button() {
  press_button(UP_BUTTON);
}

void press_down_button() {
  press_button(DOWN_BUTTON);
}

void wake_up_screen() {
  uint32_t current_millis = millis();

  if (current_millis - stored_millis >= 1000 * SCREEN_TIMEOUT_S) {
    press_menu_button();
    my_delay(SCREEN_WAKEUP_MS);  // needed to wake up the remote
  }
}

void update_stored_millis() {
  stored_millis = millis();
}

void loop(void) {
  server.handleClient();
  delay(2);
}
