
package client;

import javax.xml.bind.JAXBElement;
import javax.xml.bind.annotation.XmlElementDecl;
import javax.xml.bind.annotation.XmlRegistry;
import javax.xml.namespace.QName;


/**
 * This object contains factory methods for each 
 * Java content interface and Java element interface 
 * generated in the client package. 
 * <p>An ObjectFactory allows you to programatically 
 * construct new instances of the Java representation 
 * for XML content. The Java representation of XML 
 * content can consist of schema derived interfaces 
 * and classes representing the binding of schema 
 * type definitions, element declarations and model 
 * groups.  Factory methods for each of these are 
 * provided in this class.
 * 
 */
@XmlRegistry
public class ObjectFactory {

    private final static QName _CapitalizeResponse_QNAME = new QName("http://textConversor/", "capitalizeResponse");
    private final static QName _Capitalize_QNAME = new QName("http://textConversor/", "capitalize");

    /**
     * Create a new ObjectFactory that can be used to create new instances of schema derived classes for package: client
     * 
     */
    public ObjectFactory() {
    }

    /**
     * Create an instance of {@link CapitalizeResponse }
     * 
     */
    public CapitalizeResponse createCapitalizeResponse() {
        return new CapitalizeResponse();
    }

    /**
     * Create an instance of {@link Capitalize }
     * 
     */
    public Capitalize createCapitalize() {
        return new Capitalize();
    }

    /**
     * Create an instance of {@link JAXBElement }{@code <}{@link CapitalizeResponse }{@code >}}
     * 
     */
    @XmlElementDecl(namespace = "http://textConversor/", name = "capitalizeResponse")
    public JAXBElement<CapitalizeResponse> createCapitalizeResponse(CapitalizeResponse value) {
        return new JAXBElement<CapitalizeResponse>(_CapitalizeResponse_QNAME, CapitalizeResponse.class, null, value);
    }

    /**
     * Create an instance of {@link JAXBElement }{@code <}{@link Capitalize }{@code >}}
     * 
     */
    @XmlElementDecl(namespace = "http://textConversor/", name = "capitalize")
    public JAXBElement<Capitalize> createCapitalize(Capitalize value) {
        return new JAXBElement<Capitalize>(_Capitalize_QNAME, Capitalize.class, null, value);
    }

}
