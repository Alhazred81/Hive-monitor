#include "web_theme.h"

String htmlHead(const String& title, const String& activeTab) {
    String s = "<!DOCTYPE html><html lang='hu'><head><meta charset='utf-8'>";
    s += "<meta name='viewport' content='width=device-width, initial-scale=1.0'>";
    s += "<title>" + title + "</title>";
    s += "<link rel='stylesheet' href='/s.css'>";
    s += "</head><body>";
    s += "<nav>";
    
    // Kaptármonitor menüpontok
    s += "<a href='/'" + String(activeTab=="1"?" class='on'":"") + ">🏠 Főoldal</a>";
    s += "<a href='/config'" + String(activeTab=="2"?" class='on'":"") + ">⚙️ Beállítások</a>";
    
    s += "</nav><div class='wrap'>";
    return s;
}

String htmlFoot() {
    return "</div></body></html>";
}

void handleCss(AsyncWebServerRequest *request) {
    String css = R"css(
        :root{--bg:#05050a;--nav:#0d0d1a;--card:#141428;--txt:#e0e0e0;--txt2:#888;
        --border:#2a2a40;--accent:#4d4dff;--ok:#00cc66;--warn:#ff9900;--err:#ff3333}
        * {box-sizing:border-box;margin:0;padding:0;font-family:system-ui,-apple-system,sans-serif}
        body {background:var(--bg);color:var(--txt);font-size:14px;line-height:1.5}
        
        /* Navigáció */
        nav {background:var(--nav);display:flex;overflow-x:auto;border-bottom:1px solid var(--border);
        position:sticky;top:0;z-index:100;padding:0 8px;scrollbar-width:none}
        nav::-webkit-scrollbar{display:none}
        nav a {color:var(--txt2);text-decoration:none;padding:14px 16px;white-space:nowrap;
        font-weight:600;font-size:13px;border-bottom:2px solid transparent;transition:.2s}
        nav a:hover {color:var(--txt)}
        nav a.on {color:var(--accent);border-bottom-color:var(--accent)}
        
        /* Elrendezés */
        .wrap {max-width:800px;margin:0 auto;padding:16px;display:flex;flex-wrap:wrap;gap:16px;align-items:flex-start}
        h1 {width:100%;font-size:20px;margin-bottom:-4px;color:#fff}
        .card {background:var(--card);border:1px solid var(--border);border-radius:12px;padding:16px;
        width:100%;flex:0 0 auto}
        h2 {font-size:14px;text-transform:uppercase;letter-spacing:1px;color:var(--txt2);
        margin-bottom:12px;border-bottom:1px solid var(--border);padding-bottom:6px}
        h3 {margin-top: 0; color: #fff;}
        
        /* Form elemek */
        label {display:block;font-size:12px;color:var(--txt2);margin-bottom:4px;margin-top:4px}
        input, select, textarea {width:100%;background:#0a0a18;color:#fff;border:1px solid var(--border);
        border-radius:8px;padding:10px;margin-bottom:12px;font-size:14px;outline:none}
        input:focus, select:focus {border-color:var(--accent)}
        button {background:var(--accent);color:#fff;border:none;border-radius:8px;padding:10px 16px;
        font-size:14px;font-weight:600;cursor:pointer;width:100%;transition:.2s}
        button:hover {filter:brightness(1.1)}
        button.sec {background:transparent;border:1px solid var(--border);color:var(--txt)}
        button.sec:hover {background:var(--border)}
        
        /* Egyedi flex layoutok a keresőmezőkhöz */
        .flex-row {display:flex;gap:10px;margin-bottom:10px;}
        .flex-row select {margin-bottom:0;flex-grow:1;}
        .flex-row button {width:auto;background:#28a745;}
        .flex-row button:hover {background:#218838;}
    )css";
    request->send(200, "text/css", css);
}