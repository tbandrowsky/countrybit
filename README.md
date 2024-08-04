
# Corona
## This is the primary repository for the back end and front ends of the Country Video Games corona project.

Corona is designed to be a low code database and forms visualization stack that can either run client side, server side, or both, allowing for a fleet of applications to collaborate.  This is for Microsoft Windows but it is envisioned that mobile devices will have their own rendering front end, and eventually some browser thing may be available.

This is designed to do experimental real time modelling with forms built up as quickly as possible and then refined as needed by either editing mark up or editing C++ code to create new controls.

## Development is the Active Branch

Visual Studio Solution Structure as follows.

1.  colorparty - Color Party application package for Microsoft Store
2.  corona - Shared Source C++ project.  Partially factored to support C++20 modules but alas not quite yet.
3.  coronaserver - Database server stand alone for Corona.  This application runs from a schema file and a config.  Edit both of those, and the database changes while running.
4.  revolution - Main client EXE projects.  All the EXEs start out from Revolution, each has its own app_file that is conditionally compiled based on what product you are building for.

## Getting Started 

Revolution is a single EXE controlled by an include to produce different .EXES.  The current output is set to reparations.

1.  Build Reparations.EXE with x64 from the Revolution Project
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

## About Reparations

1.  Country Video Games is using its technology to showcase building an insurance claims system for Reparations for Black America.  Basically, we're saying that instead of blaming everyone today for the mistakes of the past people of the USA, instead, we focus on blaming the Federal Government.  So, instead of arguing over which white guy or jew or white women is responsible for all the crimes of the Federal Government, we say, the Federal government is and we transform all the gobbly-de-gook of intersectionality in socialiology into a proper graph building process in the more disciplined computer science to make that case.

2.  It is a test of the mobility of our low code platform that we can introduce entire working systems in political time, both as genuine tool for social improvement via Reparations.

## Support Me

If you are curious to see the platform evolve, like to have that Windows native low code option that produces a very tiny EXE with a very low memory footprint, or if you believe in the cause of Reparations, or are just sick of hearing about how Democrats are the champions of social justice, you can give yourself some of that ammunition to silence criticisms on the identity circle jerk by supporting a genuine solution to perhaps the gravest of American legacly problems.

Just go ahead and drop some of those coins in the bucket!  I need help!

https://gofund.me/225daa58

## Downloads that are working and samples can be found on https://countryvideogames.com



