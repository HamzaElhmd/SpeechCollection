using System.Diagnostics;
using Microsoft.AspNetCore.Mvc;
using System.Net.Sockets;
using System.Text;
using System;
using System.Text.RegularExpressions;
using speech_collection.Models;

namespace speech_collection.Controllers.ConsentForm;

public class ConsentFormController : Controller
{
	private readonly ILogger<ConsentFormController> _logger;

	public ConsentFormController(ILogger<ConsentFormController> logger)
	{
		_logger = logger;
	}


	public IActionResult Index()
	{
		return View();
	}

	[HttpPost]
	public IActionResult Authenticate([FromBody] AuthenticateRequest request)
	{

		Console.Write("Request pendning to send to registrer service.\n");
		string authDaemonAddress = "/tmp/auth.sock";

		if (string.IsNullOrEmpty(request.emailAddress) ||
				string.IsNullOrEmpty(request.gender))
		{
			return BadRequest("A required field was left empty..");
		}

		string emailPattern = @"[A-Za-z0-9.-_]+@[a-zA-Z-]+.[a-zA-Z]+";
		Regex regexPattern = new Regex(emailPattern);

		if (regexPattern.Matches(request.emailAddress).Count == 0)
		{
			return BadRequest("Invalid email address.");
		} 

		try {
			var clientSocket = new Socket(AddressFamily.Unix,
				       	SocketType.Seqpacket,
				       	ProtocolType.Unspecified);

			var authDaemon = new UnixDomainSocketEndPoint(authDaemonAddress);
			clientSocket.Connect(authDaemon);

			string requestMessage = $"SEND {request.emailAddress} {request.gender} {request.native}";
                	byte[] requestBytes = Encoding.UTF8.GetBytes(requestMessage);

			clientSocket.Send(requestBytes);

			byte[] responseBytes = new byte[1024];
			int bytesReceived = clientSocket.Receive(responseBytes);
			string responseMessage = Encoding.UTF8.GetString(responseBytes, 0, bytesReceived);

			Console.Write("Response received " + responseMessage + "\n");			       
			string? redirectUrl;

			if (responseMessage.Equals("OK"))
			{
				Console.Write("Received OK\n");
				redirectUrl = Url.Action("Verification", "ConsentForm");
			} else 
				return BadRequest("Couldn't send verification code, try again..");

			Console.Write("Redirection URL : " + redirectUrl + "\n");
			return Ok(new {redirectUrl});
		} catch (Exception ex)
		{
			Console.Write("ERROR : " + ex);

			return BadRequest("Something went wrong, try again ..");
		}
  	}

	public IActionResult Verification()
	{
		Console.Write("Different View : Verification.\n");

		return View();
	}

	[HttpPost]
	public IActionResult Validate([FromBody] ConsentCode verifCode)
	{
    		string? redirectUrl;
    		string authDaemonAddress = "/tmp/auth.sock";

		if (verifCode == null || string.IsNullOrEmpty(verifCode.verificationCode))
    		{
        		return BadRequest("Verification code is required.");
    		}

    		try {
        		var clientSocket = new Socket(AddressFamily.Unix,
                    	SocketType.Seqpacket,
                    	ProtocolType.Unspecified);

        		var authDaemon = new UnixDomainSocketEndPoint(authDaemonAddress);
       	 		clientSocket.Connect(authDaemon);

        		string requestMessage = $"VALIDATE {verifCode.verificationCode}";
        		byte[] requestBytes = Encoding.UTF8.GetBytes(requestMessage);

        		clientSocket.Send(requestBytes);

        		byte[] responseBytes = new byte[1024];
        		int bytesReceived = clientSocket.Receive(responseBytes);
        		string responseMessage = Encoding.UTF8.GetString(responseBytes, 0, bytesReceived);

        		if (responseMessage.Equals("OK"))
        		{
            			Console.Write("Received OK, redirecting to Dashboard\n");
            			// Redirect to the Dashboard controller's Index action
            			redirectUrl = Url.Action("Index", "SpeechDataCollection");
        		}
        		else
            			return BadRequest("Invalid verification code, try again");

        		Console.Write("Response Received " + responseMessage + "\n");
        		return Ok(new { redirectUrl });
    		}
	    	catch (SocketException ex)
    		{
        		Console.Write("ERROR: " + ex + "\n");
        		return BadRequest("Connection failure, try again..");
    		}
	}

}
