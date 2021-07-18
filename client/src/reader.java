import java.io.DataInputStream;

class Reader {
	private DataInputStream in;
	Reader(DataInputStream in) {
		this.in = in;
	}
	String readLine() {
		byte[] chs = new byte[256];
		int i = 0;

		do {
			try { chs[i++] = in.readByte();}
			catch (Exception e) {
				System.out.println("Exeption " + e.toString());
				e.printStackTrace();
			}
		}while(chs[i-1] != 0);
		String line = new String(chs);
		return line.substring(0, line.indexOf('\0'));
	}
}
