# Speech Collection Tool

Web application for collecting speech data from consenting users for speech recognition and phonetic analysis. Built for the Darija Pronunciation Corrector capstone project at Al Akhawayn University.

## Tech Stack

- **Framework**: ASP.NET Core (MVC)
- **Database**: MongoDB
- **Audio Processing**: FFMPEG
- **Model**: Wav2Vec2-XLSR-53 (multilingual ASR)

## Services

1. **Data Handler Service**
   - MongoDB integration for user data storage
   - Consent form management (email, nativity, nationality, gender)
   - FFMPEG audio standardization (sample rate, bit format, channels)

2. **Verification Service**
   - Email verification with 6-digit codes
   - MongoDB-backed code validation

## Author

**Hamza El Hamdi**
