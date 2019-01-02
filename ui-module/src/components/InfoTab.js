import React from 'react';
import { inject, observer } from 'mobx-react';
import Table from 'react-bootstrap/lib/Table';
import Button from 'react-bootstrap/lib/Button';
import Duration from 'duration';

const jsonbuild = process.env.JSON_BUILD;

export default
@inject('deviceStateStore')
@observer
class InfoTab extends React.Component {
    handleChange = () => {
      this.props.deviceStateStore.toggleRelay();
    };

    render() {
      const {
        relay,
        isChanging,
        uptime,
        ssid,
        ip,
        pow,
        mac,
        versionFirmware,
        deviceType,
      } = this.props.deviceStateStore;
      let relayState = relay ? 'On' : 'Off';
      relayState = deviceType === 1 ? 'Open' : relayState;
      const json = jsonbuild ? new Date(JSON.parse(jsonbuild).buildDate).toLocaleString() : 'undefined';
      return (
        <Table striped bordered condensed hover>
          <thead>
            <tr>
              <th>Name</th>
              <th>Value</th>
            </tr>
          </thead>
          <tbody>
            <tr>
              <td>{deviceType === 1 ? 'Door' : 'Relay'}</td>
              <td>
                <Button
                  bsStyle="primary"
                  disabled={isChanging}
                  onClick={!isChanging ? this.handleChange : null}
                >
                  {isChanging ? 'Changing...' : relayState}
                </Button>
              </td>
            </tr>
            <tr>
              <td>Uptime</td>
              <td>{new Duration(new Date(new Date().getTime() - uptime)).toString(1)}</td>
            </tr>
            <tr>
              <td>SSID</td>
              <td>{ssid}</td>
            </tr>
            <tr>
              <td>Ip</td>
              <td>{ip}</td>
            </tr>
            <tr>
              <td>Mac</td>
              <td>{mac}</td>
            </tr>
            <tr>
              <td>UI Version</td>
              <td>{json}</td>
            </tr>
            <tr>
              <td>POW</td>
              <td>{pow}</td>
            </tr>
            <tr>
              <td>Firmware Version</td>
              <td>{versionFirmware}</td>
            </tr>
            <tr>
              <td>Firmware</td>
              <td><a href={`http://${ip}/update`}>update</a></td>
            </tr>
          </tbody>
        </Table>
      );
    }
}
