import React from 'react';
import { inject, observer } from 'mobx-react';
import Table from 'react-bootstrap/lib/Table';


export default
@inject('deviceStateStore')
@observer
class SmartThingsTab extends React.Component {
  render() {
    const { smartthingsName, smartthingsStatus } = this.props.deviceStateStore;
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
            <td>Switch Status</td>
            <td>{smartthingsStatus}</td>
          </tr>
        </tbody>
      </Table>
    );
  }
}
