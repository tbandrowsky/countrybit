
# Corona
## This is the primary repository for the back end and front ends of the Country Video Games corona project.

Corona is designed to be a low code database and forms visualization stack that can either run client side, server side, or both, allowing for a fleet of applications to collaborate.  This is for Microsoft Windows but it is envisioned that mobile devices will have their own rendering front end, and eventually some browser thing may be available.

This is designed to do experimental real time modelling with forms built up as quickly as possible and then refined as needed by either editing mark up or editing C++ code to create new controls.

## master is the Active Branch

Visual Studio Solution Structure as follows.

1.  coronaserver - Database server stand alone for Corona.  This application runs from a schema file and a config.  Edit both of those, and the database changes while running.
2.  corona - Shared Source C++ project.  Partially factored to support C++20 modules but alas not quite yet.

3.  colorparty - Color Party application package for Microsoft Store

3.  coronawinui - Win/RT client component for corona.  The idea is to drop this control into a nuget and we'll let anyone have apps with a local database and remote detabase, including our own Revolution.

4.  revolution* - Main client EXE projects.  Revolution Win32 is the smallest possible client, and is built around Win32 native controls.  RevolutionWinUI is really the most active thing, as I love the MVVM of WinUI.

## Getting Started, Windows Server

coronaserver is the database server.  Run that, and the config file specifies the binding point annd other things for the web listener and the database server.  It's an object oriented database server with inheritance and rich querying across multiple ODBC data sources. It should start up and if you hit CTRL-C you also get the corona console.  

The default url to browse it is in the config file.

Presently, the server seems to work ok but much more testing is needed.

Be sure to read the Word document CoronaApplicationServer.docx.

## Getting Started, Windows Client

## Downloads that are working and samples can be found on https://countryvideogames.com



