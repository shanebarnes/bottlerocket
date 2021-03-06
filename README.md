![](doc/bottlerocket.png)

# bottlerocket

| [Latest][tag-link] | [License][lic-link] |
| :----------------: | :-----------------: |
| ![tag-badge]       | ![lic-badge]        |

[lic-badge]: https://img.shields.io/github/license/shanebarnes/bottlerocket.svg "License"
[lic-link]: https://github.com/shanebarnes/bottlerocket/blob/master/LICENSE "License"
[tag-badge]: https://img.shields.io/github/tag/shanebarnes/bottlerocket.svg "Latest"
[tag-link]: https://github.com/shanebarnes/bottlerocket/releases "Latest"

A multi-threaded network benchmarking tool capable of generating:
     high traffic volumes,
     large numbers of concurrent traffic flows, and
     detailed network and traffic flow analytics.

## Build Status

| [Linux][lin-link] | [Macintosh][mac-link] | [Windows][win-link] |
| :---------------: | :-------------------: | :-----------------: |
| ![lin-badge]      | ![mac-badge]          | ![win-badge]        |

[lin-badge]: https://travis-ci.org/shanebarnes/bottlerocket.svg?branch=master "Travis build status"
[lin-link]:  https://travis-ci.org/shanebarnes/bottlerocket "Travis build status"
[mac-badge]: https://travis-ci.org/shanebarnes/bottlerocket.svg?branch=master "Travis build status"
[mac-link]:  https://travis-ci.org/shanebarnes/bottlerocket "Travis build status"
[win-badge]: https://ci.appveyor.com/api/projects/status/7v1u5mbgu2acqn50/branch/master "AppVeyor build status"
[win-link]:  https://ci.appveyor.com/project/shanebarnes/bottlerocket/branch/master "AppVeyor build status"

## Installation

Without unit tests:
cmake -H. -Bbuild; cd build; make

With unit tests:
cmake -H. -Bbuild -DBR_TESTS_ENABLE=YES

## Usage

TODO: Write usage instructions

## Contributing

1. Fork it!
2. Create your feature branch: `git checkout -b my-new-feature`
3. Commit your changes: `git commit -am 'Add some feature'`
4. Push to the branch: `git push origin my-new-feature`
5. Submit a pull request :D

## History

TODO: Write history

## Credits

TODO: Write credits

## License

TODO: Write license
