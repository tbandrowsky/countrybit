
# Corona
## This is the primary repository for the back end and front ends of the Country Video Games corona project.

Corona is designed to be a low code database and forms visualization stack that can either run client side, server side, or both, allowing for a fleet of applications to collaborate.  This is for Microsoft Windows but it is envisioned that mobile devices will have their own rendering front end, and eventually some browser thing may be available.

This is designed to do experimental real time modelling with forms built up as quickly as possible and then refined as needed by either editing mark up or editing C++ code to create new controls.

A Python bridge is coming.

## Development is the Active Branch

Visual Studio Solution Structure as follows.

1.  colorparty - Color Party application package for Microsoft Store
2.  corona - Shared Source C++ project.  Partially factored to support C++20 modules but alas not quite yet.
3.  coronaserver - Database server stand alone for Corona.  This application runs from a schema file and a config.  Edit both of those, and the database changes while running.
4.  revolution - Main client EXE projects.  All the EXEs start out from Revolution, each has its own app_file that is conditionally compiled based on what product you are building for.

## Downloads that are working and samples can be found on https://countryvideogames.com



