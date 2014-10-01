Zhenhai Dashboard
=================

Architecture
-------------

  - Front-end utils

    - SementicUI CSS Framework
    - jQuery 
    - jQueryUI
    - d3.js

  - Back-end framework

    - Sails.js MVC web framework


Installation
-------------

  - Install sails global command-line program

        $ npm install -g sails

  - Install local npm modules

        $ cd dashboard/
        $ npm install

Configuration
---------------

There are two settings need to be adjusted: MachineList and EMail server configuration.

  1. MachineList: config/machines.js

        module.exports.machineList = [
          "A001", "A002", "A003", "A004", "A005",
          "A006", "A007", "A008", "A009", "A010",
          "B001", "B002", "B003", "B004", "B005",
          "B006", "B007", "B008", "B009", "B010",
          "C001", "C002", "C003", "C004", "C005",
          "C006", "C007", "C008", "C009", "C010",
          "D001", "D002", "D003", "D004", "D005",
          "D006", "D007", "D008", "D009", "D010",
          "E001", "E002", "E003", "E004", "E005",
          "E006", "E007", "E008", "E009", "E010"
        ];

  2. EMail SMTP server:

        module.exports.mail = {
          host: 'localhost',
          port: 25,
          username: 'username'
          password: 'password',
          from: "ZhenHai Dashboard <zhenhai@zhenhai.com.tw>"
        };


Start
---------------

  1. Start MongoDB server, or HTTP server will not start.
 
  2. Start HTTP server

        $ sails lift

  3. Connect to http://localhost:1337/


