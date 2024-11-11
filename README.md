
# Corona
## This is the primary repository for the back end and front ends of the Country Video Games corona project.

Corona is designed to be a low code database and forms visualization stack that can either run client side, server side, or both, allowing for a fleet of applications to collaborate.  This is for Microsoft Windows but it is envisioned that mobile devices will have their own rendering front end, and eventually some browser thing may be available.

This is designed to do experimental real time modelling with forms built up as quickly as possible and then refined as needed by either editing mark up or editing C++ code to create new controls.

## corona_express_table is the Active Branch

Visual Studio Solution Structure as follows.

1.  colorparty - Color Party application package for Microsoft Store
2.  corona - Shared Source C++ project.  Partially factored to support C++20 modules but alas not quite yet.
3.  coronaserver - Database server stand alone for Corona.  This application runs from a schema file and a config.  Edit both of those, and the database changes while running.
4.  revolution - Main client EXE projects.  All the EXEs start out from Revolution, each has its own app_file that is conditionally compiled based on what product you are building for.

## Getting Started, Windows Server

coronaserver is the database server.  Run that, and the config file specifies the binding point annd other things for the web listener and the database server.  It's an object oriented database server with inheritance and rich querying across multiple ODBC data sources. It should start up and if you hit CTRL-C you also get the corona console.  

The default url to browse it is in the config file.

Presently, the server seems to work ok but much more testing is needed.

You can:



## Getting Started, Windows Client

Revolution is a single EXE controlled by an include to produce different .EXES.  The current output is set to donationfinder.  There's a lot of damage right now, so hang tight!

1.  Build donationfinder.EXE with x64 from the Revolution Project

2.  Run It
  
In one window is the application monitoring window, through which, all things on the Corona bus are shown.
a.  Parse errors from configuration files
b.  Run time events and bus interactions
c.  Consistent json output

3.  Fire up your favorite editor and pay close attention to these files.
a.  schema.json  You can edit this and change the database schema while the app is running.  Schema specifies a set of classes in the system.  Classes may be derived via single inheritance.


b.  pages and style.json.  You can edit this to change the appearance and bindings of the page to the command bus.


## Downloads that are working and samples can be found on https://countryvideogames.com



