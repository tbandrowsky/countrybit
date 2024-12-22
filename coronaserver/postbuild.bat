
DEL  /Q .\distribution\*.*
COPY ..\x64\Release\coronaserver.exe .\distribution\coronaserver.exe
COPY ..\x64\Release\CoronaApplicationServer.docx .\distribution\CoronaApplicationServer.docx
COPY ..\x64\Release\*.json .\distribution\
COPY dockerfile .\distribution\
docker build -t tbandrow/corona .