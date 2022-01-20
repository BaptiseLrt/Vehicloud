package org.eclipse.om2m.app;
 
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.InetSocketAddress;
import java.util.concurrent.Executors;
 
import org.json.JSONArray;
import org.json.JSONObject;
 
import com.sun.net.httpserver.HttpExchange;
import com.sun.net.httpserver.HttpHandler;
import com.sun.net.httpserver.HttpServer;
 
public class MyMonitor {
 
	private static String originator="admin:admin";
	private static String cseProtocol="http";
 
	private static String aeProtocol="http";
	private static String aeIp = "192.168.1.23";
	private static int aePort = 8080;	
	private static String targetCse = "in-cse/in-name";
 
	private static String appPoa = aeProtocol+"://"+aeIp+":"+aePort;
 
	private static JSONObject ae;
	private static JSONObject sub;	
 
	public static void main(String[] args) {
 
		HttpServer server = null;
		try {
			server = HttpServer.create(new InetSocketAddress(1600), 0);
		} catch (IOException e) {
			e.printStackTrace();
		}
		server.createContext("/", new MyHandler());
		server.setExecutor(Executors.newCachedThreadPool());
		server.start();
 
	}
 
	static class MyHandler implements HttpHandler {
 
		public void handle(HttpExchange httpExchange)  {
			System.out.println("Event Recieved!");
 
			try{
				InputStream in = httpExchange.getRequestBody();
 
				String requestBody = "";
				int i;char c;
				while ((i = in.read()) != -1) {
					c = (char) i;
					requestBody = (String) (requestBody+c);
				}
 
				System.out.println(requestBody);
 
				JSONObject json = new JSONObject(requestBody);
				if (json.getJSONObject("m2m:sgn").has("m2m:vrq")) {  
					System.out.println("Confirm subscription");
				} else {
					JSONObject rep = json.getJSONObject("m2m:sgn").getJSONObject("m2m:nev")
							.getJSONObject("m2m:rep").getJSONObject("m2m:cin");
					
					int ty = rep.getInt("ty");
					System.out.println("Resource type: "+ty);
 
				if (ty == 4) {
                        String ciName = rep.getString("rn");
                        String content = rep.getString("con");

                        System.out.println("[INFO] New Content Instance " + ciName + " has been created");
                        System.out.println("[INFO] Content: " + content);
                        System.out.println("[INFO] Copying data in in-cse:");

                        // POST CIN DATA

                        JSONObject obj = new JSONObject();
                        obj.put("cnf","application/json");
                        obj.put("con", content);
                        JSONObject cin = new JSONObject();
                        cin.put("m2m:cin", obj);
                        RestHttpClient.post(originator, appPoa + "/~/" + targetCse + "/" + "DATA_RECEP" + "/DATA" , cin.toString(), 4);
                    }
				}	
 
				String responseBudy ="";
				byte[] out = responseBudy.getBytes("UTF-8");
				httpExchange.sendResponseHeaders(200, out.length);
				OutputStream os = httpExchange.getResponseBody();
				os.write(out);
				os.close();
 
			} catch(Exception e){
				e.printStackTrace();
			}		
		}
	}
}