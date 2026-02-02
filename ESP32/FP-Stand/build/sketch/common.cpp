#line 1 "/home/marwan/Projects/Embedded/Arduino/ESP32/FP-Stand/common.cpp"
#include "Arduino.h"
#include "ESPAsyncWebServer.h"
#include "HardwareSerial.h"
#include "common.h"
#include <SHA1Builder.h>
#include <ArduinoJson.h>

void handlePortal(AsyncWebServerRequest *request);
void handleRoot(AsyncWebServerRequest *request);
void handleNotFound(AsyncWebServerRequest *request);
void handleLogin(AsyncWebServerRequest *request);
void handleLogout(AsyncWebServerRequest *request);
void handleAdminData(AsyncWebServerRequest *request);
void handleLang(AsyncWebServerRequest* request);
void handleConfig(AsyncWebServerRequest* request);
void handleGen204(AsyncWebServerRequest *request);

typedef bool(*loaderFunc)(char* key,char* value) ;

bool parseConfig(loaderFunc loader);
bool baseConfigLoader(char* key,char* value);

File html_index ;
File config_file ;

DNSServer dnsServer;
AsyncWebServer server(80);
AsyncEventSource ev_sink("/events");


struct ServerConfig{
    char* password = nullptr;
}server_config;

loaderFunc base_loader = baseConfigLoader;
loaderFunc second_pass_loader = nullptr;


bool initializeCommon(){
    Serial.begin(115200);

    if(!SPIFFS.begin(true)){
        Serial.println("An Error has occurred while mounting SPIFFS");
        return false;
    }

    Serial.print("SPIFFS bytes: ");
    Serial.println(SPIFFS.usedBytes());

    html_index = SPIFFS.open("/index.html");
    if(!html_index){
        Serial.println("Failed to open \"index.html\" file for reading");
        return false;
    };

    config_file = SPIFFS.open("/config.txt");
    if(!config_file){
        Serial.println("Failed to open \"config.txt\" file for reading");
        return false;
    };

    {
        unsigned long t_abs = micros();
        if(!parseConfig(base_loader)){
            Serial.println("Failed to parse \"config.txt\"");
            return false;
        };
        unsigned long t0 = micros();

        Serial.print("Parse took ");
        Serial.print(t0 - t_abs);
        Serial.println(" us");
    }

    WiFi.setTxPower(WIFI_POWER_19_5dBm);
    if (!WiFi.softAP(ssid, password)) {
        log_e("Soft AP creation failed.");
        return false;
    }
    Serial.println("AP MODE");
    //WiFi.mode(WIFI_STA);
    //WiFi.begin("B.H4703","rachid1973");

    //WiFi.AP.enableDhcpCaptivePortal();

    if (dnsServer.start()) {
        Serial.println("Started DNS server in captive portal-mode");
    } else {
        Serial.println("Err: Can't start DNS server!");
    }

    return true;
};

void attachServerHandlers(){
    server.on("/", handleRoot);
    server.on("/portal", handlePortal);
    server.on("/admin/login", handleLogin);
    server.on("/admin/logout", handleLogout);
    server.on("/admin/data", handleAdminData);
    server.on("/lang",handleLang);
    server.on("/lang/popup",handleLang);
    server.on("/langs",[](AsyncWebServerRequest* request){
        File root = SPIFFS.open("/lang");
        size_t buf_size = 5;
        char* langs = (char*)malloc(buf_size);
        memset(langs,0,buf_size);
        memcpy(langs,"[",1);
        size_t files = 0;
        File file = root.openNextFile();
        while (file) {
            if (!file.isDirectory() && ("/lang/"+String(file.name()) == file.path())) {
                Serial.println(file.path());
                if(files > 0){
                    langs[(files*5)+1+4] = ',';
                }
                langs = (char*) realloc(langs,buf_size+=5);
                memcpy(langs+(files*5)+1,R"("""" )",5);
                memcpy(langs+(files*5)+1+1,file.name(),2);
                files++;
            }
            file = root.openNextFile();
        }
        memcpy(langs+(buf_size-4),"]\0",2);

        request->send(200,"application/json",langs);
        return;
    });
    server.on("/admin",[](AsyncWebServerRequest* request){
        request->send(isAuthenticated(request),"","");
    });
    server.onNotFound(handleNotFound);

    //server.on("/generate_204", HTTP_ANY,handleGen204);

    //server.on("/gen_204", HTTP_ANY,handleGen204);

    server.addHandler(&ev_sink);

};


void beginServer(){
    server.begin();

    Serial.println("AP + web server started!");

};

void handleGen204(AsyncWebServerRequest *request){
    // HEAD requests are only probes
    if (request->method() == HTTP_HEAD) {
        request->send(204);  // pure probe
        return;
    }

    // Mini-browser detection (no Chrome in UA)
    if (request->hasHeader("User-Agent")) {
        String ua = request->getHeader("User-Agent")->value();
        if (ua.indexOf("Chrome") == -1) {
            AsyncWebServerResponse *res =
                request->beginResponse(302, "text/html", "");
            res->addHeader("Location", "/portal");
            request->send(res);
            return;
        }
    }

    // Standard probe reply
    request->send(200, "text/html", "<html><body>OK</body></html>");

};

void handleRoot(AsyncWebServerRequest *request) {
    request->send(200, "text/html", "<h1>ESP32 GUI</h1>");
}

void handleNotFound(AsyncWebServerRequest *request) {
    AsyncWebServerResponse *res =
        request->beginResponse(302, "text/html", "");
    res->addHeader("Location", "/portal");
    request->send(res);
};


void handlePortal(AsyncWebServerRequest *request){
    request->send(SPIFFS,"/index.html","text/html");
};

void handleLogin(AsyncWebServerRequest* request){
    if(!request->hasParam("pwd")){
        request->send(400,"","");
        return;
    };
    AsyncWebParameter* pwd = request->getParam("pwd");
    String pwd_str = pwd->value();
    SHA1Builder hasher;

    if(pwd_str == String(server_config.password) ){
        hasher.begin();
        hasher.add(pwd_str.c_str());
        hasher.add(":");
        hasher.add(request->client()->remoteIP().toString());
        hasher.calculate();
        AsyncWebServerResponse* response = request->beginResponse(202,"","");
        response->addHeader("Location","/");
        response->addHeader("Cache-Control","no-cache");
        response->addHeader(
            "Set-Cookie",
            "SESSIONID=" + hasher.toString() + "; Path=/admin; HttpOnly"
        );

        request->send(response);
    }else{
        Serial.print(pwd_str);
        Serial.print(" != ");
        Serial.println(String(server_config.password));
        request->send(406,"","");
    }

};

void handleLogout(AsyncWebServerRequest *request){
    AsyncWebServerResponse* response = request->beginResponse(200,"","");
    response->addHeader("Location","/");
    response->addHeader("Cache-Control","no-cache");
    response->addHeader(
        "Set-Cookie",
        "SESSIONID=deleted; Max-Age=0; Path=/admin; HttpOnly"
    )
    ;
    request->send(response);
    return;
};

int32_t isAuthenticated(AsyncWebServerRequest *request){
    if(!request->hasHeader("Cookie"))
        return 400;
    String cookie = request->getHeader("Cookie")->value();
    SHA1Builder hasher;
    hasher.begin();
    hasher.add(server_config.password);
    hasher.add(":");
    hasher.add(request->client()->remoteIP().toString());
    hasher.calculate();

    if(cookie.indexOf("SESSIONID=" + hasher.toString()) != -1){
        return 202;
    }

    return 401;
};




void handleLang(AsyncWebServerRequest* request){
    if(!request->hasParam("lang")){
        request->send(400,"","");
        Serial.println("NOPE");
        return;
    };

    AsyncWebParameter* lang_param = request->getParam("lang");

    String lang = lang_param->value();
    String path = "";
    if(request->url().endsWith("popup")){
        path = "/lang/popup/"+lang+".json";
    }else{
        path = "/lang/"+lang+".json";
    }

    if(!SPIFFS.exists(path)){
        request->send(400,"text/html",path+" not found");
    }


    request->send(SPIFFS,path,"application/json");
};

void handleConfig(AsyncWebServerRequest* request){
    request->send(404,"text/html","Not yet implemented");
    return;
    WebRequestMethodComposite method = request->method();
    if(method != HTTP_GET && method != HTTP_PATCH){
        request->send(400,"","");
        return;
    }
    if(method == HTTP_PATCH){



    }else if(method == HTTP_GET){

    }


};



bool parseConfig(loaderFunc loader){
    if (!config_file) return false;

    const size_t maxLine = 256;     // big enough for ANY config line
    char line[maxLine];
    size_t idx = 0;

    config_file.seek(0);

    while (true) {
        int c = config_file.read();
        if (c < 0) break;  // EOF

        if (c == '\n') {
            line[idx] = '\0';

            // parse key=value
            char *sep = strchr(line, '=');
            if (sep) {
                *sep = '\0';
                if(!loader(line, sep + 1) && second_pass_loader){
                    second_pass_loader(line, sep +1);
                }
            }

            idx = 0;
            continue;
        }

        if (idx < maxLine - 1) {
            line[idx++] = (char)c;
        }
    }

    // process last line if no newline at EOF
    if (idx > 0) {
        line[idx] = '\0';
        char *sep = strchr(line, '=');
        if (sep) {
            *sep = '\0';
            loader(line, sep + 1);
        }
    }

    return true;
}

bool baseConfigLoader(char* key,char* value){
    size_t v_size = strlen(value);
    if(strcmp(key,"password") == 0){
        server_config.password = (char*) malloc(v_size+1);
        memcpy(server_config.password,value,v_size+1);
        Serial.print("Server PASS = ");
        Serial.println(server_config.password);
        return true;
    }
    return false;
};

void setConfigLoader(bool (*loader)(char* key,char* value),bool overwrite){
    if(overwrite){
        base_loader = loader;
    }else{
        second_pass_loader = loader;
    };
};

AsyncEventSource& getEventSink(){
    return ev_sink;
};
