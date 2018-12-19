import React from 'react';
import { inject, observer } from 'mobx-react';
import Table from 'react-bootstrap/lib/Table';
import Button from 'react-bootstrap/lib/Button';


export default @inject('deviceStateStore', 'neighborsStore')

@observer
class NeighborsTab extends React.Component {
    handleChange = (ip, status) => {
      let newStatus = status;
      if (status === 'on') {
        newStatus = 'off';
      } else if (status === 'off') {
        newStatus = 'on';
      }
      console.log(`ip=${ip}, status=${status}`);
      this.props.neighborsStore.requestChangeState(ip, newStatus);
    };

    render() {
      const { smartthingsDevices } = this.props.deviceStateStore;
      const { devicePending, devicesStatus } = this.props.neighborsStore;
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

                    Object.entries(smartthingsDevices).map((map) => {
                      const { ip } = map[1].ip;
                      const isPending = devicePending[ip];
                      const st = map[1].status;
                      const ds = devicesStatus[ip] ? devicesStatus[ip].status : st;
                      const status = ds || st;
                      return (
                        <tr>
                          <td><a href={`http://${map[1].ip}`}>{map[1].ip}</a></td>
                          <td>{map[0]}</td>
                          <td>
                            {(new Date().getTime() - map[1].lastTime) > (1000 * 60 * 30)
                              ? 'Offline' : 'Online'}
                          </td>
                          <td>{map[1].name}</td>
                          <td>
                            <Button
                              bsStyle="primary"
                              disabled={isPending}
                              onClick={() => this.handleChange(map[1].ip, status)}
                            >
                              {isPending ? 'changing' : status}
                            </Button>
                          </td>
                          <td><a href={`http://${map[1].ip}/update`}>upgrade</a></td>
                        </tr>);
                    })}


          </tbody>
        </Table>
      );
    }
}
