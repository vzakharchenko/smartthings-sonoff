import React from 'react';
import { inject, observer } from 'mobx-react';
import Table from 'react-bootstrap/lib/Table';


export default
@inject('deviceStateStore')
@observer
class SmartThingsTab extends React.Component {
  render() {
    const { smartthingsName, smartthingsStatus, deviceType } = this.props.deviceStateStore;
    let statusString = 'Switch Status';
    let smartthingsStatusString = smartthingsStatus;
    if (deviceType === 1) {
      statusString = 'Door Status';
      smartthingsStatusString = smartthingsStatus === 'on' ? 'OK' : 'Failed';
    }
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
            <td>Device Name</td>
            <td>{smartthingsName}</td>
          </tr>
          <tr>
            <td>{statusString}</td>
            <td>{smartthingsStatusString}</td>
          </tr>
        </tbody>
      </Table>
    );
  }
}
