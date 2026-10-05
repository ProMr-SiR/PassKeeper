#pragma once
#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>
#include "../Core/clsPasswordManager.h"
#include "../Core/clsRandomPasswordGenerator.h"

/**
 * @class clsWebDashboard
 * @brief Manages the local web server and Captive Portal for the settings dashboard.
 * Provides a dynamic HTML interface for adding, editing, and deleting 
 * password entries over Wi-Fi without needing external apps or internet access.
 */
class clsWebDashboard {
private:
    WebServer _server;     // The HTTP Web Server listening on port 80
    DNSServer _dnsServer;  // DNS server to redirect all requests to the Captive Portal
    bool _isActive;        // Status flag to check if the dashboard is running
    const byte DNS_PORT = 53;

    /**
     * @brief Helper function to escape HTML special characters.
     * Prevents XSS vulnerabilities and HTML breakage if passwords contain `<` or `"`.
     */
    static String htmlEscape(String input) {
        input.replace("&", "&amp;");
        input.replace("<", "&lt;");
        input.replace(">", "&gt;");
        input.replace("\"", "&quot;");
        return input;
    }

    /**
     * @brief Generates the static HTML header including inline CSS.
     * Uses a modern dark UI design to provide a clean dashboard aesthetic.
     * @param title The page title to be displayed in the browser tab.
     */
    String _getHtmlHeader(String title) {
        String html = "<!doctype html><html><head><meta name='viewport' content='width=device-width,initial-scale=1'>";
        html += "<meta charset='UTF-8'>";
        html += "<title>" + title + "</title>";
        html += "<style>";
        html += "body{margin:0;font-family:Arial,sans-serif;background:#0b0b0b;color:#f5f5f5;padding:22px;}";
        html += ".wrap{max-width:980px;margin:0 auto;}";
        html += "h1{margin:0 0 18px 0;text-align:center;font-size:28px;font-weight:700;color:#ffffff;}";
        html += ".card{background:#161616;border:1px solid rgba(255,255,255,0.08);border-radius:20px;padding:24px;margin-bottom:22px;box-shadow:0 10px 24px rgba(0,0,0,0.35);} ";
        html += "h2{margin:0 0 18px 0;font-size:18px;color:#ffffff;}";
        html += "label{display:block;margin:14px 0 8px 0;font-weight:600;color:#d7d7d7;}";
        html += "input{width:100%;padding:14px 16px;border-radius:12px;border:1px solid rgba(255,255,255,0.10);background:#0f0f0f;color:#fff;box-sizing:border-box;font-size:15px;outline:none;}";
        html += "input::placeholder{color:#8d8d8d;}";
        html += ".actions{display:flex;gap:12px;flex-wrap:wrap;margin-top:22px;}";
        html += ".btn{display:inline-block;padding:13px 22px;border:none;border-radius:12px;font-size:15px;font-weight:700;text-decoration:none;cursor:pointer;text-align:center;}";
        html += ".btn-primary{background:#4d8ef7;color:#fff;}";
        html += ".btn-secondary{background:#4b4b4b;color:#fff;}";
        html += ".btn-edit{background:#4d8ef7;color:#fff;padding:10px 18px;border-radius:10px;text-decoration:none;}";
        html += ".btn-delete{background:#ef4b3f;color:#fff;padding:10px 18px;border-radius:10px;text-decoration:none;display:inline-block;}";
        html += "table{width:100%;border-collapse:collapse;overflow:hidden;border-radius:14px;}";
        html += "th,td{padding:16px 14px;border:1px solid rgba(255,255,255,0.08);text-align:left;vertical-align:middle;}";
        html += "th{background:#202020;color:#f2f2f2;font-size:15px;}";
        html += "td{background:#161616;}";
        html += ".table-actions{display:flex;gap:10px;flex-wrap:wrap;align-items:center;}";
        html += ".muted{color:#bdbdbd;font-size:14px;margin-top:8px;}";
        html += ".grid-menu { display: grid; grid-template-columns: repeat(auto-fit, minmax(200px, 1fr)); gap: 15px; }";
        html += ".menu-item { background:#161616; border:1px solid rgba(255,255,255,0.08); border-radius:15px; padding:20px; text-align:center; text-decoration:none; color:white; font-size:20px; font-weight:bold; transition: background 0.3s; }";
        html += ".menu-item:hover { background:#252525; }";
        html += "@media (max-width:700px){body{padding:14px;} .card{padding:18px;} th,td{padding:12px 10px;font-size:14px;} .table-actions{flex-direction:column;align-items:stretch;}}";
        html += "</style></head><body><div class='wrap'>";
        return html;
    }

    String _getHtmlFooter() {
        return "</div></body></html>";
    }

    /**
     * @brief Route handler for the Root page ("/").
     * Displays a list of saved services as a grid menu.
     */
    void _handleRoot() {
        String html = _getHtmlHeader("Mr.PassKaper Dashboard");
        html += "<h1>Dashboard</h1>";
        
        // Retrieve all unique service names to display as cards
        vector<string> serviceNames = clsPasswordManager::GetUniqueServiceNames();
        
        if (serviceNames.empty()) {
            html += "<div class='card'><h2>Welcome!</h2>";
            html += "<p class='muted'>No accounts saved yet. Add your first service below.</p></div>";
        } else {
            html += "<div class='grid-menu'>";
            for (const auto& name : serviceNames) {
                int count = clsPasswordManager::GetEntriesByService(name).size();
                // Link to the service management page passing the service name as a parameter
                html += "<a href='/service?name=" + String(name.c_str()) + "' class='menu-item'>" + String(name.c_str());
                html += " <span style='font-size:12px;color:#aaa;'>(" + String(count) + ")</span></a>";
            }
            html += "</div>";
        }

        // Form for opening an existing service or creating a new one quickly
        html += "<div class='card' style='margin-top:20px;'><h2>Add New Service</h2>";
        html += "<form method='GET' action='/service'>";
        html += "<label>Service Name</label>";
        html += "<input name='name' placeholder='e.g. Twitter, Discord, Steam...' required>";
        html += "<div class='actions'><button class='btn btn-primary' type='submit'>Open Service</button></div>";
        html += "</form></div>";

        html += _getHtmlFooter();
        _sendHtml(html);
    }

    /**
     * @brief Route handler for the specific Service management page ("/service").
     * Displays all accounts under a service and provides forms for Add/Edit.
     */
    void _handleService() {
        String serviceName = "Google";
        if (_server.hasArg("name")) {
            serviceName = _server.arg("name");
        }

        int editID = -1;
        if (_server.hasArg("edit")) {
            editID = _server.arg("edit").toInt(); // Check if we are in Edit mode
        }

        String formName = serviceName;
        String formUser = "";
        String formPass = "";
        bool isEditMode = (editID >= 0);

        // Populate form data if in Edit mode
        if (isEditMode) {
            clsPasswordManager entry = clsPasswordManager::FindByID(editID);
            if (!entry.IsEmpty()) {
                formName = htmlEscape(entry.Name().c_str());
                formUser = htmlEscape(entry.Username().c_str());
            } else {
                isEditMode = false;
            }
        }

        // If adding a new account, generate a strong default password suggestion
        if (!isEditMode) {
            formPass = clsRandomPasswordGenerator::GenerateWord(clsRandomPasswordGenerator::MixCharsAndSpecial, 16).c_str();
        }

        String html = _getHtmlHeader(serviceName + " Accounts");
        html += "<h1>" + serviceName + " Management</h1>";
        
        // --- Add/Edit Form Section ---
        html += "<div class='card'><h2>";
        html += isEditMode ? "Edit Account" : "Add New Account";
        html += "</h2><form method='POST' action='/save'>";

        if (isEditMode) {
            html += "<input type='hidden' name='edit_id' value='" + String(editID) + "'>";
        }

        html += "<input type='hidden' name='item_name' value='" + serviceName + "'>";
        html += "<label>Username / Email</label>";
        html += "<input name='item_user' placeholder='test@gmail.com' value='" + formUser + "' required>";
        html += "<label>Password</label>";
        html += "<input id='pwd' name='item_pass' type='text' placeholder='Enter password' value='" + formPass + "' required>";

        // Password length slider integration using inline JS
        html += "<div style='margin-top:16px;'>";
        html += "<div style='margin-bottom:8px;color:#cfcfcf;'>Password Length: <span id='lenVal'>16</span></div>";
        html += "<input id='len' type='range' min='4' max='64' value='16' style='width:100%;'>";
        html += "<div style='display:flex;justify-content:space-between;color:#8d8d8d;font-size:12px;'><span>4</span><span>64</span></div>";
        html += "<div style='margin-top:10px;'>";
        html += "<button type='button' class='btn btn-secondary' onclick='gen()'>Regenerate</button>";
        html += "</div></div>";

        // Client-side JavaScript to generate passwords dynamically without reloading
        html += "<script>";
        html += "const cs='ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz23456789!@#$%^&*';";
        html += "function gen(){var l=parseInt(document.getElementById('len').value)||16;var s='';for(var i=0;i<l;i++){s+=cs[Math.floor(Math.random()*cs.length)];}document.getElementById('pwd').value=s;document.getElementById('lenVal').textContent=l;}";
        html += "document.addEventListener('DOMContentLoaded',function(){var r=document.getElementById('len');var v=document.getElementById('lenVal');if(r&&v){v.textContent=r.value;r.addEventListener('input',function(){v.textContent=this.value;});}});";
        html += "</script>";

        html += "<div class='actions'>";
        html += "<button class='btn btn-primary' type='submit'>";
        html += isEditMode ? "Save Changes" : "Add Account";
        html += "</button>";
        if (isEditMode) {
            html += "<a class='btn btn-secondary' href='/service?name=" + serviceName + "'>Cancel</a>";
        }
        html += "<a class='btn btn-secondary' href='/' style='margin-left:auto;'>Back to Home</a>";
        html += "</div></form></div>";

        // --- Saved Accounts Table Section ---
        html += "<div class='card'><h2>Saved " + serviceName + " Accounts</h2>";
        
        vector<clsPasswordManager> vEntries = clsPasswordManager::GetEntriesList();
        
        int serviceCount = 0;
        for (const auto& e : vEntries) {
            if (e.Name() == string(serviceName.c_str())) serviceCount++;
        }

        if (serviceCount == 0) {
            html += "<p class='muted'>No saved accounts yet.</p>";
        } else {
            html += "<table><tr><th>Username</th><th>Actions</th></tr>";
            for (const auto& entry : vEntries) {
                if (entry.Name() == string(serviceName.c_str())) {
                    html += "<tr>";
                    html += "<td>" + htmlEscape(entry.Username().c_str()) + "</td>";
                    html += "<td><div class='table-actions'>";
                    html += "<a class='btn-edit' href='/service?name=" + serviceName + "&edit=" + String(entry.ID()) + "'>Edit</a>";
                    // Delete button with a JavaScript confirmation dialog
                    html += "<a class='btn-delete' href='/delete_now?id=" + String(entry.ID()) + "&name=" + serviceName + "' onclick=\"return confirm('Are you sure you want to delete this account?');\">Delete</a>";
                    html += "</div></td>";
                    html += "</tr>";
                }
            }
            html += "</table>";
        }
        html += "</div>";

        html += _getHtmlFooter();
        _sendHtml(html);
    }

    /**
     * @brief HTTP POST handler for saving/updating credentials ("/save").
     * Extracts form data and updates the ESP32 NVS memory.
     */
    void _handleSave() {
        String itemName = _server.arg("item_name");
        String itemUser = _server.arg("item_user");
        String itemPass = _server.arg("item_pass");
        
        itemName.trim();
        itemUser.trim();
        itemPass.trim();

        if (itemUser.length() == 0 || itemPass.length() == 0) {
            _server.send(400, "text/plain", "Username and Password are required!");
            return;
        }

        if (_server.hasArg("edit_id")) {
            // Update existing account
            int editID = _server.arg("edit_id").toInt();
            clsPasswordManager entry = clsPasswordManager::FindByID(editID);
            if (!entry.IsEmpty()) {
                entry.SetUsername(itemUser.c_str());
                entry.SetPassword(itemPass.c_str());
                entry.Save();
            }
        } else {
            // Insert entirely new account
            clsPasswordManager::AddNewEntry(itemName.c_str(), itemUser.c_str(), itemPass.c_str());
        }

        // Redirect browser back to the service management page
        String redirectUrl = "/service?name=" + itemName;
        _server.sendHeader("Location", redirectUrl);
        _server.send(303);
    }

    /**
     * @brief Route handler for deleting an account ("/delete_now").
     * Deletes the requested entry ID from memory and redirects back.
     */
    void _handleDelete() {
        String serviceName = "Google";
        if (_server.hasArg("name")) {
            serviceName = _server.arg("name");
        }

        if (_server.hasArg("id")) {
            int deleteID = _server.arg("id").toInt();
            clsPasswordManager::DeleteEntry(deleteID);
        }

        String redirectUrl = "/service?name=" + serviceName;
        _server.sendHeader("Location", redirectUrl);
        _server.send(303);
    }

    /**
     * @brief Sends the compiled HTML response to the connected client.
     * Includes headers to explicitly disable caching so data is always fresh.
     */
    void _sendHtml(const String& html) {
        _server.sendHeader("Cache-Control", "no-store, no-cache, must-revalidate, max-age=0");
        _server.sendHeader("Pragma", "no-cache");
        _server.sendHeader("Expires", "-1");
        _server.send(200, "text/html", html);
    }

public:
    clsWebDashboard() : _server(80), _isActive(false) {}

    /**
     * @brief Initializes the Wi-Fi Access Point and Web Server stacks.
     * @param ssid The Wi-Fi Network Name.
     * @param password The Wi-Fi Network Password.
     * @return bool True if successfully started.
     */
    bool Start(const char* ssid, const char* password) {
        // Stop any previous connections and reset state
        WiFi.disconnect(true, true);
        WiFi.softAPdisconnect(true);
        delay(200);
        
        // Configure ESP32 into Access Point (AP) mode
        WiFi.mode(WIFI_AP);
        delay(100);

        bool apStarted = WiFi.softAP(ssid, password);
        if (!apStarted) return false;

        delay(300);

        // Start the DNS Server to hijack all domains (*) and redirect to our IP
        _dnsServer.start(DNS_PORT, "*", WiFi.softAPIP());

        // Setup primary server routes
        _server.on("/", HTTP_GET, [this]() { this->_handleRoot(); });
        _server.on("/service", HTTP_GET, [this]() { this->_handleService(); });
        _server.on("/save", HTTP_POST, [this]() { this->_handleSave(); });
        _server.on("/delete_now", HTTP_GET, [this]() { this->_handleDelete(); });
        
        // Captive Portal standard spoofing routes (Android & iOS triggers)
        _server.on("/generate_204", HTTP_GET, [this]() { this->_handleRoot(); });
        _server.on("/hotspot-detect.html", HTTP_GET, [this]() { this->_handleRoot(); });
        _server.onNotFound([this]() { this->_handleRoot(); });

        _server.begin();
        _isActive = true;
        return true;
    }

    /**
     * @brief Safely shuts down the web server and disables the Wi-Fi AP 
     * to save battery and prevent unauthorized access.
     */
    void Stop() {
        if (!_isActive) return;
        _dnsServer.stop();
        _server.stop();
        WiFi.softAPdisconnect(true);
        WiFi.mode(WIFI_OFF);
        _isActive = false;
    }

    /**
     * @brief Must be called constantly inside the main loop()
     * to handle active client HTTP requests and DNS queries.
     */
    void ProcessNextRequest() {
        if (!_isActive) return;
        _dnsServer.processNextRequest();
        _server.handleClient();
    }
};
