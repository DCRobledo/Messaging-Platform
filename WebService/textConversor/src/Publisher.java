package textConversor;
import javax.xml.ws.Endpoint;
import java.net.InetAddress;  
import java.net.UnknownHostException;  

public class Publisher {
	public static void main(String[] args) {
		try {
			final String url = "http://"+ InetAddress.getLocalHost().getHostName() + ":8888/rs";
			System.out.println("Publishing at " +url);
			Endpoint.publish(url, new TextConversorService());
		} catch(UnknownHostException e) {
			e.printStackTrace();
		}
		
	}
}
