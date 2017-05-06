# beame-agent-cpp

## Background

`beame-insta-ssl` is a heavy toolkit due to its nodejs dependency. To relatively simplify the product we started developing a device agent, target specifically for embedded devices with or without Linux.

This is supposed to be pure CPP 11 with boost and just a few libraries to make development reasonable. The project is meant to compile with a cross chain therefore it includes all of its dependencies, i.e. openssl, boost, librapidjson, and libwebscoket of one kind or another.

## Building
	cmake .
	make
