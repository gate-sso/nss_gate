# nss\_gate

nss_gate is Unix/Linux Name Service Switch providing user, groups and shadow lookup over http using gate-sso server. GateSSO in turn uses Google MFA to authenticate - so you can actually allow MFA on Linux prompt using nss_gate.

This module enables you to authenticate users against any http based server so that you don't have to create new users on each box and centraly control user access.

* Provides user lookup over http
* Provides group lookup over http

This module was forked from opensource nss_http implementation, but this has grown into much more configurable. Given that it has only similarities about the way nss functions are implemented, this needs a new home, now it's here complementing our gate mfa server as a nice nss module.

Configuration & installation
-------------

* Check out the module from this repo
* In the `libnss_http` directory:

	```
	make
	sudo make install
	```
* You'll need to have `jansson`, `yaml` and `curl` libraries and development headers installed. eg for Ubuntu

	`sudo apt-get install libcurl4-openssl-dev libjansson-dev libyaml-dev`

* You may have to modify Makefile for appropriate destination

* Once the module is compiled and installed, please place `libnss_http.so.2` into `/lib/x86_64*/` on ubuntu or find where other nss modules are installed and put it over there.

* create directory /etc/gate and put nss.yml with following configuration

	```yaml
	   		nss_http:
	  			host_url: https://<gate-sso-server>/nss
	  			api_key: <EXAMPLE TOKEN>
	```
* Now if you issue getent passwd - then you should see remote users in the list
* You should run check_nss and it should print configuration for nss_http


## Old Description

    This is a simple Name Service Switch module that I wrote in order to quickly
    extend nss via an HTTP server that runs on localhost. __This is seriously not
    meant to be used in a production environment. Not only is it inefficient but
    I'm extremely rusty with C and there are likely overflow errors and other
    security problems. Use at your own enjoyment.__
    
The above is old description

I forked to make two changes.

1. Include YAML support so that we can read from yaml file instead of hardcoded server
2. Include path support so that we can get what we want over public URL but with token - so that it can be secure.

With this module, I made it generic and now it hits our own opensource mfa server to give user's appropriate access.

## Installation

In the `libnss_http` directory:
```
make
sudo make install
```

You'll need to have `libjansson` and `libcurl` installed.

`sudo apt-get install libcurl4-openssl-dev libjansson-dev libyaml-dev`

## Configuration

Once the module is installed on your system you just have to modify the
`/etc/nsswitch.conf` for the appropriate service you want to utilize the 
http server on. The service name is `http` and currently supports the `passwd`, `group`,
`shadow` databases.

e.g. `/etc/nsswitch.conf`
```
passwd:         http compat
group:          http compat
shadow:         http compat
```

## API

By default `nss_http` expects a server to be listening on `127.0.0.1:9669`. This is compiled in
and can be modified in the nss_http.h file. I have provided an example server in the `examples`
directory though I'll describe the API below.

### Endpoints

#### /passwd

The `/passwd` endpoint should return an array of password objects as described below:

```javascript
[
    {
        pw_name: "gary",
        pw_passwd: "x",
        pw_uid: 100,
        pw_gid: 100,
        pw_gecos: "User Information",
        pw_dir: "/home/gary",
        pw_shell: "/bin/bash"
    }
]
```

See `man 3 getpwent` for more information about the passwd struct.

The passwd endpoint should expect to receive 1 of 2 mutually exclusive query parameters. `name`,
containing a username or `uid` containing a user id. If either of these query parameters are
received, a single object should be returned for the requested username/uid.


#### /group

The `/group` endpoint should return an array of group objects as described below:

```javascript
[
    {
        gr_name: "users",
        gr_passwd: "x",
        gr_gid: 101,
        gr_mem: ["gary", "carolyn"]
    }
]
```

See `man 3 getgrent` for more information about the group struct.

The group endpoint should expect to receive 1 of 2 mutually exclusive query parameters. `name`,
containing a username or `gid` containing a group id. If either of these query parameters are
received, a single object should be returned for the requested groupname/gid.

#### /shadow

The `/shadow` endpoint should return an array of shadow objects as described below:

```javascript
[
    {
        sp_namp: "gary",
        sp_pwdp: "<encrypted password>",
        sp_lstchg: 16034,
        sp_min: 0,
        sp_max: 99999,
        sp_warn: 7,
        sp_inact: null,
        sp_expire: null,
        sp_flag: null
    }
]
```

See `man 3 getspent` for more information about the shadow struct.

The shadow endpoint should expect to receive a query parameter of `name`,
containing a username. If this query parameter is received, a single object
 should be returned for the requested username.

## Sources

There were a lot of sources involved with completion of this project but the major
ones were the following:

 * http://www.gnu.org/software/libc/manual/html_node/Name-Service-Switch.html
 * https://code.google.com/p/nsscache/
 * https://github.com/akheron/jansson/
 * http://curl.haxx.se/libcurl/c/
