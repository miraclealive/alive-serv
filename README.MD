## [Help translate this README into your language!](https://github.com/miraclealive/alive-serv/pulls)

# alive-serv 
  
This is a **heavily WIP** game server implementation for SIF2 MIRACLE LIVE!, whose last remaining server shut down on May 31st, 2024. It is written in C, and is the product of reverse engineering the server protocol of the global version.

## Status

Five endpoints are currently implemented and functional:

* `/maintenance/maintenance.json`: static file that returns the maintenance status of the server
* `/api/start/assetHash`: returns the current asset version hash to the client
* `/api/dummy/login`: returns the user_id resolved from client-provided info and initiates the login process
* `/api/start`: returns the token resolved from the user_id along with the previously provided asset version hash
* `/api/debug/error`: receives and logs a debug message from the client in the case of a catastrophic error

These five endpoints currently enable the client to get to the title screen and a bit further into the login sequence.

This server begun development in early February 2024 using Global version 1.0.2 and is still currently only tested against this version. I expect this server to work for later clients; including the Japanese versions, with little changes. However, this remains untested for now.

## Building

In-depth building instructions are to come later. However, it should be fairly straightforward if you know how to use CMake. This server makes use of the following libraries:

* [ulfius](https://github.com/babelouest/ulfius)
* [jansson](https://github.com/akheron/jansson)
* [orcania](https://github.com/babelouest/orcania)
* [OpenSSL](https://github.com/openssl/openssl)
* [MariaDB](https://github.com/mariadb-corporation/mariadb-connector-c)

## Usage

Once built, the server can be properly invoked by providing the following command line arguments in order:

`<server_port> <db_host> <db_port> <db_username> <db_password> <db_name>`

Currently, the server will automatically set up the necessary tables in your specified MariaDB database if they don't already exist. As the database specification is rapdily changing, you will likely need to drop all tables regularly to continue using the server properly.

The server can be connected to using a custom proxy on an unmodified game client. Alternatively, you can apply patches to the client itself; however, this is something I have yet to explore.

## Why C?

You may be wondering why I chose to write this server in pure C when there may be objectively better options for RESTful game servers such as this, like ASP.NET or FastAPI. There's absolutely nothing wrong with using these. The short answer is that I wanted to write an ambitious project in pure C as a fun challenge. I have written game servers in the past using frameworks in higher-level languages, so I thought I would go lower-level for this project to make it more interesting to work on.

## License

This code is licensed under the MIT license. Please see [LICENSE](LICENSE) for more information.

## Disclaimer

I'm building this server for free, purely driven by a strong passion for video game preservation and reverse engineering. As this is a hobby, progress may be slow and inconsistent or nonexistent altogether at times. Please keep this in mind.
