import React from 'react';
import { inject, observer } from 'mobx-react';
import Table from 'react-bootstrap/lib/Table';


export default
@inject('deviceStateStore')
@observer
class NeighborsTab extends React.Component {
  render() {
    const { smartthingsDevices } = this.props.deviceStateStore;
    return (
      <Table striped bordered condensed hover>
        <thead>
          <tr>
            <th>Ip</th>
            <th>Mac</th>
            <th>Health Status</th>
            <th>Name</th>
            <th>Status</th>
            <th>Firmware</th>
          </tr>
        </thead>
        <tbody>
          {

                Object.entries(smartthingsDevices).map(map => (
                  <tr>
                    <td><a href={`http://${map[1].ip}`}>{map[1].ip}</a></td>
                    <td>{map[0]}</td>
                    <td>
                      {(new Date().getTime() - map[1].lastTime) > (1000 * 60 * 30)
                        ? 'Offline' : 'Online'}
                    </td>
                    <td>{map[1].name}</td>
                    <td>{map[1].status}</td>
                    <td><a href={`http://${map[1].ip}/update`}>upgrade</a></td>
                  </tr>
                ))}


        </tbody>
      </Table>
    );
  }
}
