# NetworkActivityObserver (C++ and Python 3.12)
Client-Server App to monitor network activities of users on their computers

# Description:
Two-side app to monitor any activity from user to server, by sending POST request on server. When user uses mouse or keyboard, any activity from
hardware logs on server, there you can see full information of last activity. NetworkObserver can take screenshots and write them to default 'screenshot' folder (if present).
In case if server is down, there's a backup desktop App that continues montoring on users. Also does logs of activities & shows list of connected users online.

# How to use this project:
For client-side app, just run it. Located inside 'клиентская часть' folder. For server, follow this steps:
1) Download Python 3.12. https://www.python.org/downloads/release/python-3120/ Install Pip library;
2) Install by -pip command next libraries: OpenCV, cURL, nlohman-json. Alternatively you download them from these URL links:
cURL -> https://curl.se/download.html
OpenCV -> https://github.com/opencv/opencv
Json-Nlohman -> https://json.nlohmann.me/home/releases/
4) Compile server by "Debug->Run" or "Run by Debugging" option.
