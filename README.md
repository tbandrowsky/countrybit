
# Corona
## This is the primary repository for the back end and front ends of the Country Video Games corona project.

Corona is designed to be a low code database and forms visualization stack that can either run client side, server side, or both, allowing for a fleet of applications to collaborate.  This is for Microsoft Windows but it is envisioned that mobile devices will have their own rendering front end, and eventually some browser thing may be available.

This is designed to do experimental real time modelling with forms built up as quickly as possible and then refined as needed by either editing mark up or editing C++ code to create new controls.

## skip_list is the Active Branch

Visual Studio Solution Structure as follows.

1.  colorparty - Color Party application package for Microsoft Store
2.  corona - Shared Source C++ project.  Partially factored to support C++20 modules but alas not quite yet.
3.  coronaserver - Database server stand alone for Corona.  This application runs from a schema file and a config.  Edit both of those, and the database changes while running.
4.  revolution - Main client EXE projects.  All the EXEs start out from Revolution, each has its own app_file that is conditionally compiled based on what product you are building for.

## Getting Started, Windows Server

coronaserver is the database server.  Run that, and the config file specifies the binding point annd other things for the web listener and the database server.  It's an object oriented database server with inheritance and rich querying.  It will be pretty sweet as soon as it works! LOL.  But it should start up and if you hit CTRL-C you also get the corona console.  

## Getting Started, Windows Client

Revolution is a single EXE controlled by an include to produce different .EXES.  The current output is set to donationfinder.  There's a lot of damage right now, so hang tight!

1.  Build donationfinder.EXE with x64 from the Revolution Project
  ![image](https://github.com/user-attachments/assets/99357bc6-5da6-4482-8941-0aa5f28e0668)

2.  Run It

  <img width="1244" alt="image" src="https://github.com/user-attachments/assets/e76bfb6b-22b5-41a1-843f-f69ad5f1e51d">

In one window is the application monitoring window, through which, all things on the Corona bus are shown.
a.  Parse errors from configuration files
b.  Run time events and bus interactions
c.  Consistent json output

3.  Fire up your favorite editor and pay close attention to these files.
a.  schema.json  You can edit this and change the database schema while the app is running.  Schema specifies a set of classes in the system.  Classes may be derived via single inheritance.

![image](https://github.com/user-attachments/assets/68bfbe10-3879-4699-b60c-dae0278baf19)

b.  pages and style.json.  You can edit this to change the appearance and bindings of the page to the command bus.
![image](https://github.com/user-attachments/assets/5a22b93c-963d-43cf-a67e-ba58c28e8a86)


## Downloads that are working and samples can be found on https://countryvideogames.com



