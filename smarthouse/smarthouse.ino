#include <WiFi.h> // Ucitavanje WiFi biblioteke

const char* ssid     = "SatKom";
const char* password = "123456789";

// Port 80 je standardni HTTP port koji se često koristi za komunikaciju između web servera i klijenata.
WiFiServer server(80);

// Varijabla za storiranje HTTP zahtjeva
String header;
String output26State = "off";
String output27State = "off";

// GPIO pinovi koji pokrecu blinkanje LED lampi
const int output26 = 26;
const int output27 = 27;

void setup() {
  Serial.begin(115200); //inicijalizacija serijske komunikacije između mikrokontrolera i računara. Podešava se brzina prenosa podataka na 115200 bps. 
// postavljanje izlaznih pinova
  pinMode(output26, OUTPUT);
  pinMode(output27, OUTPUT);
  // iskljucivanje povezanih uredjaja
  digitalWrite(output26, LOW);
  digitalWrite(output27, LOW);

  // konfiguracija uređaja kao pristupne tačke za wifi mrežu.
  Serial.print("Setting AP (Access Point)…");

  WiFi.softAP(ssid, password);

  IPAddress IP = WiFi.softAPIP(); //IP adresu pristupne tačke smejšta se u promenljivu IP
  Serial.print("AP IP address: ");
  Serial.println(IP);
  
  server.begin(); // ova linija koda pokreće server za WiFi komunikaciju
}

void loop(){
  WiFiClient client = server.available();   //provjera da li postoji dolazna konekcija klijenta na serveru, ako postoji objekat client nije prazan

  if (client) {                             
    Serial.println("New Client.");          // ispisuje se poruka na serijskom portu da je klijent povezan sa serverom
    String currentLine = "";                // String za pohranu vrijednosti korisnika
    while (client.connected()) {            //  obrada podataka koji stižu od klijenta i petlja traje sve dok je klijent povezan
      if (client.available()) {             // provjera da li postoje dostupni podaci za čitanje od klijenta i metoda client.available() vraća broj dostupnih bajtova za čitanje
        char c = client.read();             // čita se jedan bajt podataka iz klijenta i smijesta u varijablu c
        Serial.write(c);                    // ispisivanje na serijski port
        header += c;                        // dodavanje podataka u string
        if (c == '\n') {                    // u slucaju newline karaktera, znači da je završen redak HTTP zahteva koji je poslao klijent
          
          if (currentLine.length() == 0) {
            // Ova linija koda provjerava da li je currentLine string prazan
             // ako jest to znači da je prethodni redak bio prazan i da je to kraj HTTP zahteva te server šalje odgovor klijentu
    
            client.println("HTTP/1.1 200 OK"); //HTTP statusni odgovor "200 OK" salje se klijentu što znači da je zahtjev uspješno primljen i obrađen
            client.println("Content-type:text/html"); // salje se HTTP zaglavlje koje definiše tip sadržaja kao "text/html" što znači da će se odgovor servera tretirati kao HTML tekst
            client.println("Connection: close"); //zatvaranje konekcije nakon slanja odgovora
            client.println(); //prazan red koji oznacava kraj HTTP zaglavlja
            
            // paljenje i gasenje GPIO pinova
            // provjerava se sadržaj header stringa kako bi se identifikovalo koji zahtjev je poslao klijent
            if (header.indexOf("GET /26/on") >= 0) {
              Serial.println("GPIO 26 on");
              output26State = "on";
              digitalWrite(output26, HIGH);
            } else if (header.indexOf("GET /26/off") >= 0) {
              Serial.println("GPIO 26 off");
              output26State = "off";
              digitalWrite(output26, LOW);
            } else if (header.indexOf("GET /27/on") >= 0) {
              Serial.println("GPIO 27 on");
              output27State = "on";
              digitalWrite(output27, HIGH);
            } else if (header.indexOf("GET /27/off") >= 0) {
              Serial.println("GPIO 27 off");
              output27State = "off";
              digitalWrite(output27, LOW);
            }
            
            //HTML izgled aplikacije
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");
            client.println("<style>html { font-family: Helvetica; background-color: #b1ddd3; display: inline-block; margin: 0px auto; text-align: center;}");
            client.println(".button { background-color: #00b588; border: none; color: white; padding: 16px 40px;");
            client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
            client.println(".button2 {background-color:#e51111;}</style></head>");
            

           
            client.println("<body><h1>Smart House</h1>");
                       
            
            
            client.println("<p>GPIO 26 - State " + output26State + "</p>");
                 
            if (output26State=="off") {
              client.println("<p><a href=\"/26/on\"><button class=\"button\">UKLJUCI</button></a></p>");
            } else {
              client.println("<p><a href=\"/26/off\"><button class=\"button button2\">ISKLJUCI</button></a></p>");
            } 
               
            
            client.println("<p>GPIO 27 - State " + output27State + "</p>");
                
            if (output27State=="off") {
              client.println("<p><a href=\"/27/on\"><button class=\"button\">UKLJUCI</button></a></p>");
            } else {
              client.println("<p><a href=\"/27/off\"><button class=\"button button2\">ISKLJUCI</button></a></p>");
            }        
                client.println("</body></html>");
            
            
            client.println();

            break;
          } else { 
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }
      }
    }
    // ocistimo string header
    header = "";
    // zatvranje konekcije
    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");
  }
}