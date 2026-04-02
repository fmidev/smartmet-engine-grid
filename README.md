# smartmet-engine-grid

Part of [SmartMet Server](https://github.com/fmidev/smartmet-server). See the [SmartMet Server documentation](https://github.com/fmidev/smartmet-server) for a full overview of the ecosystem.

## Introduction

The Grid Engine allows SmartMet Plugins to access services related to the grid support. This mean in practice that they can access information stored into the Content Information Storage by using **the Content Server API**, fetch grid data from the grid files by using **the Data Server API** or make queries by using **the Query Server API**. 

All these APIs can be found from the Grid Engine. At the moment, the Grid-GUI Plugin and the Timeseries Plugin are using these services. In future, also the WFS Plugin and the Download Plugin should be able to use these services.

The figure below shows the basic usage of the Grid Engine.  

![Image](https://github.com/fmidev/smartmet-tools-grid/blob/master/doc/grid-support-img/grid-support-img5.png "gui")

In this configuration there is an external Content Information Storage (= Redis database) that contains the grid content information, meanwhile the actual grid information management is concentrated into the Grid Engine. In other words, the (Caching) Content Server, the Data Server and the Query Server are embedded into the Grid Engine. 

## Configuration
The main configuration file of the Grid Engine is read only once when the server is started. The main configuration file of the SmartMet server should point to this file. 

When the actual implementations of the grid services are embedded into the Grid Engine (i.e. not used remotely), all these services must be configured when the Grid Engine is configured. In this case the main configuration file contains a lot of references to other configuration files (mapping files, LUA files, alias files, etc.). These configuration files can be changed during the runtime and changes will be automatically loaded without any restart.

The Grid Engine can be configured also in a such way that all grid services are used remotely. In this case the Content Server API, the Data Server API and the Query Server API are client implementations that communicate with the remote services.

## Documentation

- [Grid Engine documentation](https://github.com/fmidev/smartmet-engine-grid/blob/master/doc/grid-engine.md)
- [Quick Setup](https://github.com/fmidev/smartmet-tools-grid/blob/master/doc/quick-setup.md)
- [Grid Support overview](https://github.com/fmidev/smartmet-tools-grid/blob/master/doc/grid-support.md)

## License

MIT — see [LICENSE](LICENSE)

## Contributing

Bug reports and pull requests are welcome on [GitHub](../../issues).
