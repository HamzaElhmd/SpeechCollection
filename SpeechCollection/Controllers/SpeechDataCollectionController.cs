using System.Diagnostics;
using Microsoft.AspNetCore.Mvc;
using System.Net.Sockets;
using System.Text;
using System;
using speech_collection.Models;

namespace speech_collection.Controllers;

public class SpeechDataCollectionController : Controller
{
	private readonly ILogger<SpeechDataCollectionController> _logger;
	private readonly string daemonAddress = "/tmp/auth.sock";

	public SpeechDataCollectionController(ILogger<SpeechDataCollectionController> logger)
	{
		_logger = logger;
	}

	public IActionResult Index()
	{
		var model = new AudioUploadViewModel();

		return View(model);
	}

	[HttpPost]
	public IActionResult UploadFile(IFormFile audioFile)
	{
		var model = new AudioUploadViewModel();

    		if (audioFile != null && audioFile.Length > 0)
    		{
        		Console.Write("Received file from user.\n");

        		var result = SendFile(audioFile);

        		if (result == "OK")
        		{
				model.successMessage = "Successfully uploaded file";
            			return View("Index", model);
        		}
        		else if (result.Equals("ERROR"))
        		{
            			model.errorMessage = "A problem occured when sending the file. File size is too large.";
				return View("Index", model);
        		}
		}

		model.errorMessage = "No file specified.";
		return View("Index", model);
	}

	[HttpPost]
	public IActionResult Record(IFormFile audio)
	{
    		if (audio == null || audio.Length == 0)
    		{
        		Console.Write("Audio is null or empty.\n");
        		return Json(new { messageLog = "ERROR: empty audio" });
    		}

    		var result = SendFile(audio);

    		if (result == "OK")
    		{
        		return Json(new { messageLog = "File successfully sent." });
    		}
    		else
    		{
        		return BadRequest("Error occured while sending recording, try again.");
    		}
	}

	private string SendFile(IFormFile file)
	{
    		try
    		{
        		using (var clientSocket = new Socket(AddressFamily.Unix, SocketType.Seqpacket, ProtocolType.Unspecified))
			{
				var daemon = new UnixDomainSocketEndPoint(daemonAddress);
            			clientSocket.Connect(daemon);

            			Console.Write("Connected to pands daemon.\n");

            			string requestMessage = $"UPLOAD {file.FileName} {file.Length}";
            			byte[] requestBytes = Encoding.UTF8.GetBytes(requestMessage);

            			clientSocket.Send(requestBytes);
            			Console.Write("Wrote to pands daemon.\n");

            			byte[] responseBytes = new byte[1024];
            			var bytesReceived = clientSocket.Receive(responseBytes);

            			string responseMessage = Encoding.UTF8.GetString(responseBytes, 0, bytesReceived);
            			Console.Write($"Received Response: {responseMessage}\n");

            			if (responseMessage.Equals("OK"))
            			{
                			using (Stream fileStream = file.OpenReadStream())
                			{
                    				byte[] fileBytes = new byte[file.Length];
                    				int count = fileStream.Read(fileBytes, 0, (int)file.Length);

                    				if (count <= 0)
                    				{
                        				return "ERROR: Couldn't read file.";
                    				}

                    				clientSocket.Send(fileBytes);
                    				Console.Write($"Successfully wrote {count} bytes to daemon.\n");

						byte[] responseBytes_ = new byte[1024];
						var bytesReceived_ = clientSocket.Receive(responseBytes_);
						string responseMessage_ = Encoding.UTF8.GetString(responseBytes_, 0, bytesReceived_);
						Console.Write($"Response Message : {responseMessage_}\n");
						if (responseMessage.Equals("OK"))
                    					return "OK";
						else 
							return "ERROR";
					}
				}
            			else
            			{
                			return "ERROR";
            			}
			}
		}
		catch (SocketException ex)
		{
			Console.Write("Error: " + ex + "\n");
        		return "ERROR";
    		}
	}
}
