#include <WiFi.h>

const int servoPin = 13;  /* pin 13 */

const char* ssid = "slic"; /* Add your router's SSID */
const char* password = "slic0072"; /*Add the password */

int dutyCycle = 0;
//int position1 = 0;

/* Setting PWM properties */
const int PWMFreq = 50;
const int PWMChannel = 0;
const int PWMResolution = 8;
const int MAX_DUTY_CYCLE = (int)(pow(2, PWMResolution) - 1);

WiFiServer espServer(80); /* Instance of WiFiServer with port number 80 */
/* 80 is the Port Number for HTTP Web Server */

/* A String to capture the incoming HTTP GET Request */
String request;

void setup()
{  
  Serial.begin(115200);
  ledcSetup(PWMChannel, PWMFreq, PWMResolution);
  /* Attach the LED PWM Channel to the GPIO Pin */
  ledcAttachPin(servoPin, PWMChannel);
  ledcWrite(PWMChannel, dutyCycle);

  Serial.print("\n");
  Serial.print("Connecting to: ");
  Serial.println(ssid);
  WiFi.mode(WIFI_STA); /* Configure ESP32 in STA Mode */
  WiFi.begin(ssid, password); /* Connect to Wi-Fi based on the above SSID and Password */
  while(WiFi.status() != WL_CONNECTED)
  {
    Serial.print("*");
    delay(100);
  }
  Serial.print("\n");
  Serial.print("Connected to Wi-Fi: ");
  Serial.println(WiFi.SSID());
  delay(100);
  /* The next four lines of Code are used for assigning Static IP to ESP32 */
  /* Do this only if you know what you are doing */
  /* You have to check for free IP Addresses from your Router and */
  /* assign it to ESP32 */
  /* If you are comfortable with this step, */
  /* please un-comment the next four lines and make necessary changes */
  /* If not, leave it as it is and proceed */
  //IPAddress ip(192,168,1,6);   
  //IPAddress gateway(192,168,1,1);   
  //IPAddress subnet(255,255,255,0);   
  //WiFi.config(ip, gateway, subnet);
  delay(2000);
  Serial.print("\n");
  Serial.println("Starting ESP32 Web Server for Servo Control...");
  espServer.begin(); /* Start the HTTP web Server */
  Serial.println("ESP32 Servo Web Server Started");
  Serial.print("\n");
  Serial.print("The URL of ESP32 Servo Web Server is: ");
  Serial.print("http://");
  Serial.println(WiFi.localIP());
  Serial.print("\n");
  Serial.println("Use the above URL in your Browser to access ESP32 Servo Web Server\n");
}
void loop()
{
  WiFiClient client = espServer.available(); /* Check if a client is available */
  if(!client)
  {
    return;
  }

  Serial.println("New Client!!!");
  boolean currentLineIsBlank = true;
  while (client.connected())
  {
    if (client.available())
    {
      char c = client.read();
      request += c;
      Serial.write(c);
        /* If you've gotten to the end of the line (received a newline */
        /* character) and the line is blank, the http request has ended, */
        /* so you can send a reply */
      if (c == '\n' && currentLineIsBlank)
      {
        client.println("HTTP/1.1 200 OK");
        client.println("Content-type:text/html");
        client.println("Connection: close");
        client.println();

        client.println("<!DOCTYPE html>");
        client.println("<html>");
        
        client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
        client.println("<link rel=\"icon\" href=\"data:,\">");

        /* CSS Styling for Text and Slider */
        
        client.println("<style>body { font-family: \"Courier New\"; margin-left:auto; margin-right:auto; text-align:center;}");
        
        client.println(".slidecontainer { width: 100%;}");
        client.println(".button { -webkit-appearance: none;");
        client.println("width: 30%; height: 20px; background: #d3d3d3;");
        client.println("outline: none; opacity: 0.7; -webkit-transition: .2s; transition: opacity .2s;}");
        client.println(".button:hover { opacity: 1; }");

        client.println(".button::-webkit-button-thumb { -webkit-appearance: none;");
        client.println("appearance: none; width: 15px; height: 28px;");
        client.println("border-radius: 30%; background: #4CAF50; cursor: pointer;}");
        client.println(".button::-moz-range-thumb { width: 25px; height: 25px; background: #4CAF50; cursor: pointer;}</style>");
        
        client.println("<script src=\"https://code.jquery.com/jquery-3.6.0.min.js\"></script>");
        /*Actual Web Page */
        client.println("</head><body><h2>Remote Lock Control</h2>");
        client.println("<p>Click the button to unlock/lock the door.</p>");
        
        client.println("<input type=\"button\" min=\"0\" max=\"180\" class=\"button\" id=\"servoRange\" onclick=\"servo.write(180)\"/>");
        client.println("<p>Angle: <span id=\"servoPos\"></span></p>");
        client.println("<script>");
        client.println("var button = document.getElementById(\"servoRange\");");
        client.println("var output = document.getElementById(\"servoPos\");");
        client.println("output.innerHTML = button.value;");
        client.println("button.oninput = function(){output.innerHTML = 180;}");
        client.println("$.ajaxSetup({timeout:1000}); function servo(angle) { ");
        client.println("$.get(\"/servovalue=\" + angle); {Connection: close};}</script>");
                
        client.println("</body></html>");   
        
        /* The request will be in the form of 
         * GET /servovalue=143 /HTTP/1.1*/
        if(request.indexOf("GET /servovalue=") != -1)
        {
          int position1 = request.indexOf('='); /* Find out the position of '=' in the request string */
          String angleStr = request.substring(position1+1); /* Next 2/3 characters inform the desired angle */
          int angleValue = angleStr.toInt();
          dutyCycle = map(angleValue, 0, 180, 0, 180);
          ledcWrite(PWMChannel, dutyCycle); 
        }
        client.println();
        break;
      }

        if(c == '\n')
        {
          currentLineIsBlank = true;
        }
        else if(c != '\r')
        {
          currentLineIsBlank = false;
        }
        //client.print("\n");
    }
  }
 
  delay(1);
  request = "";
  //client.flush();
  client.stop();
  Serial.println("Client disconnected");
  Serial.print("\n");
}