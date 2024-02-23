## [Help translate this README into your language!](https://github.com/miraclealive/alive-serv/pulls)

# alive-serv 
  
This is a **heavily WIP** game server implementation for SIF2 MIRACLE LIVE!, whose last remaining server will close down on May 31st, 2024. It is written in C, and is the product of reverse engineering the server protocol of the global version.

## Status

Three endpoints are currently implemented and functional:

* `/maintenance/maintenance.json`: static file that returns the maintenance status of the server
* `/api/start/assetHash`: returns the current asset version hash to the client
* `/api/dummy/login`: returns the user_id resolved from client-provided info and initiates the login process

These three endpoints currently enable the client to get to the title screen.

Although it is currently untested, this server should work with the Japanese client as well.

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

## License

This code is licensed under the MIT license. Please see [LICENSE](LICENSE) for more information.

## Disclaimer

I'm building this server for free out of a strong passion for video game preservation. At times, progress may be slow and inconsistent. If you would just like to play a local version of SIF, it is recommended that you set up a SIF1 server instead, as open source server implementations are much more complete for that game.
