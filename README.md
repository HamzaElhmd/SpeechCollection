## What is the Speech Collection Tool

The web application started as a primary component of the **Darija Pronunciation Corector** capstone project, under the supervision of Dr. Violetta Cavalli-Sforza, at Al Akhawayn University.
The speech collection tool aims at streamlining the collection of speech data from consenting users and volunteers for deep neural networks projects concerned with speech recognition, and phonetic analysis.

## Components

The web application follows the ASP .NET web framework, using an MVC (Model - View - Controller) architecture. 

The application is composed of two services :
  1. **Data Handler Service** : Concerns two main functionalities
     * MongoDB module : Implements the MongoDB C driver for general storage of user data.
     * Consent form module : Store user data entered in the consent form in a specified MongoDB database, including the following fields : Email address, Native or Non-Native speaker, Nationality, and Gender (Male - Female).
     * Audio processing module : Convert speech data received from the client into a standard format (Sample rate, bit format, channels, audio file format) using ffmpeg.
  2. **Verification Code Service** : Sends a verification code to a specified email address upon signal from the Data Handler Service, and verifies them in the MongoDB database that stores randomly generated 6 digit codes.
