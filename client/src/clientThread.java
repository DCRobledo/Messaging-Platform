import java.io.DataInputStream;
import java.io.IOException;
import java.net.ServerSocket;
import java.net.Socket;

class clientThread extends Thread {
	ServerSocket sc;
	boolean must_run;
	

	public clientThread(ServerSocket sc){
		this.sc = sc;
	}

	private void closeConnection(Socket sc) {
		try {
			sc.close();
		} catch (IOException e) {
			System.err.println("Exception " + e.toString());
			e.printStackTrace();
			return;
		}
	}

	public void run() {
		Reader reader = null;
		Socket socket = null;
			
		while(true) {
			try {
				socket = this.sc.accept();
				DataInputStream in = new DataInputStream(socket.getInputStream());
				reader = new Reader(in);
			} catch (IOException e) {
				if(must_run) {
					System.err.println("Exception " + e.toString());
					e.printStackTrace();
				}
				return;
			}

			// We start reading message until we get the expected format -> SEND_MESSAGE\0<id>\0<sender>\0<message>\0
			String line = reader.readLine();

			// And process the input in that case
			if (line.equals("SEND_MESSAGE"))  {
				String output = "MESSAGE ";
				output += reader.readLine() +" FROM ";
				output += reader.readLine() +":\n\t";
				output += reader.readLine();
				System.out.println(output);
				System.out.print("\tEND\nc> "); 
				closeConnection(socket);

			} else if (line.equals("SEND_MESS_ACK")) {
				closeConnection(socket);
			} else {
				System.out.println("Llega: "+line);
			}
		}

	}
}