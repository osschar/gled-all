import org.apache.axis.AxisFault;
import org.apache.axis.client.Call;
import org.apache.axis.client.Service;

import javax.xml.namespace.QName;
import javax.xml.rpc.ParameterMode;

import lia.ws.Result;

import java.io.FileReader;
import java.io.BufferedReader;
import java.io.FileWriter;

public class MonaClient
{
    public static void main(String [] args) throws Exception
    {
	// args[0] ~ service url
	// args[1] ~ command-pipe (read)
	// args[2] ~ output-pipe (write)

	FileWriter outfifo = new FileWriter(args[2]);

	// create the service
        Service  service   = new Service();
	// create the call
        Call     flv_call  = (Call) service.createCall();
        Call     v_call    = (Call) service.createCall();
        QName    qn        = new QName( "http://ws.lia", "Result" );

        flv_call.registerTypeMapping(Result.class, qn,
				     new org.apache.axis.encoding.ser.BeanSerializerFactory(Result.class, qn),    
				     new org.apache.axis.encoding.ser.BeanDeserializerFactory(Result.class, qn));
        v_call.registerTypeMapping(Result.class, qn,
				   new org.apache.axis.encoding.ser.BeanSerializerFactory(Result.class, qn),    
				   new org.apache.axis.encoding.ser.BeanDeserializerFactory(Result.class, qn));
        Result[] result;
	
        try {
            flv_call.setTargetEndpointAddress( new java.net.URL(args[0]) );
            flv_call.setOperationName( new QName("MLWebService", "getFilteredLastValues") );
	    flv_call.addParameter( "in0",org.apache.axis.encoding.XMLType.XSD_STRING , ParameterMode.IN );
	    flv_call.addParameter( "in1",org.apache.axis.encoding.XMLType.XSD_STRING , ParameterMode.IN );
	    flv_call.addParameter( "in2",org.apache.axis.encoding.XMLType.XSD_STRING , ParameterMode.IN );
	    flv_call.addParameter( "in3",org.apache.axis.encoding.XMLType.XSD_STRING , ParameterMode.IN );
            flv_call.setReturnType( org.apache.axis.encoding.XMLType.SOAP_ARRAY );

            v_call.setTargetEndpointAddress( new java.net.URL(args[0]) );
            v_call.setOperationName( new QName("MLWebService", "getValues") );	    
            v_call.addParameter( "in0",org.apache.axis.encoding.XMLType.XSD_STRING , ParameterMode.IN );
            v_call.addParameter( "in1",org.apache.axis.encoding.XMLType.XSD_STRING , ParameterMode.IN );
            v_call.addParameter( "in2",org.apache.axis.encoding.XMLType.XSD_STRING , ParameterMode.IN );
            v_call.addParameter( "in3",org.apache.axis.encoding.XMLType.XSD_STRING , ParameterMode.IN );	    
            v_call.addParameter( "in4",org.apache.axis.encoding.XMLType.XSD_LONG , ParameterMode.IN );
            v_call.addParameter( "in5",org.apache.axis.encoding.XMLType.XSD_LONG , ParameterMode.IN );
            v_call.setReturnType( org.apache.axis.encoding.XMLType.SOAP_ARRAY );
        } catch (Exception e) {
	    outfifo.write ("ERR\nInitialization error.\n");
	    outfifo.flush();
	    e.printStackTrace ();
	    return;
	}

	outfifo.write ("OK\n");
	outfifo.flush();
	
	BufferedReader cmdfifo = new BufferedReader(new FileReader(args[1]));

	while (true) {
	    try {
	    	System.err.println("going to read...");
		String cmd = cmdfifo.readLine();
		System.err.println("just read: "+cmd);
		if(cmd == null){
		    System.err.println("input closed");
		    return;
		}
		if (cmd.equals("GetFLValues")) {
		    result = (Result[]) flv_call.invoke(new Object[]
			{cmdfifo.readLine(), cmdfifo.readLine(),
			 cmdfifo.readLine(), cmdfifo.readLine()});
		} else if (cmd.equals("GetValues")) {
		    System.err.println("invokin the service...");
		    result = (Result[]) v_call.invoke(new Object[]
			{cmdfifo.readLine(), cmdfifo.readLine(),
			 cmdfifo.readLine(), cmdfifo.readLine(),
			 new Long(Long.parseLong(cmdfifo.readLine())),
			 new Long(Long.parseLong(cmdfifo.readLine()))});
		    System.err.println("invoked the service!");
		} else {
		    throw new Exception("Unknown command: " + cmd);
		}
		if (result == null ){
		    outfifo.write("ERR\nSome error occured in accessing the database.\n");
		    return;
		}
		
		// Count results
		int results = 0;
		for (int i=0;i<result.length;i++) {
		    results += result[i].getParam_name().length;
		}
		outfifo.write ("OK\n" + results+"\n");
	
		// Dump results
		for (int i=0;i<result.length;i++) {
		    for (int j=0;j<result[i].getParam_name().length;j++) {	
			outfifo.write (result[i].getFarmName()+"\n");
			outfifo.write (result[i].getClusterName()+"\n");
			outfifo.write (result[i].getNodeName()+"\n");
			outfifo.write (result[i].getParam_name()[j]+"\n");
			outfifo.write (result[i].getParam()[j].toString()+"\n");
			outfifo.write (String.valueOf(result[i].getTime())+"\n");
		    }
		} //for
		outfifo.flush();
		    
	    } catch (AxisFault fault) {
		if(fault.getFaultString().equals("No Results Found")) {
		    outfifo.write ("OK\n0\n");
		} else {
		    fault.printStackTrace();
		    outfifo.write ("ERR\n" + fault.getFaultString()+"\n");
		}
		outfifo.flush();
	    } catch (Exception e) {
		outfifo.write ("ERR\nGeneral exception.\n");
		outfifo.flush();
		e.printStackTrace ();
		return;
	    }
	}
    }
}
