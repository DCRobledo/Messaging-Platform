package textConversor;
import javax.jws.WebService;
import javax.jws.WebMethod;

@WebService public class TextConversorService {
	@WebMethod
	public String capitalize(String input) {
		String words[] = input.toLowerCase().split(" ");
		for (int i = 0; i< words.length; i++)
			words[i] = words[i].substring(0,1).toUpperCase() + words[i].substring(1);
		return String.join(" ", words);
	}
}
