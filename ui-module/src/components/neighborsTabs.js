import React from 'react';
import { inject, observer } from 'mobx-react';
import Table from 'react-bootstrap/lib/Table';
import Button from 'react-bootstrap/lib/Button';


export default @inject('deviceStateStore', 'neighborsStore')

@observer
class NeighborsTab extends React.Component {
    handleChange = (ip) => {
      this.props.neighborsStore.requestChangeState(ip);
    };

    render() {
      const { smartthingsDevices, deviceType } = this.props.deviceStateStore;
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
                      const { ip, lastTime, name } = map[1];
                      const isPending = devicePending[ip];
                      const st = map[1].status;
                      const ds = devicesStatus[ip] ? devicesStatus[ip].status : st;
                      const status = ds || st;
                      const statusString = deviceType === 1 ? 'Open' : status;
                      return (
                        <tr>
                          <td><a href={`http://${ip}`}>{ip}</a></td>
                          <td>{map[0]}</td>
                          <td>
                            {(new Date().getTime() - lastTime) > (1000 * 60 * 30)
                              ? 'Offline' : 'Online'}
                          </td>
                          <td>{name}</td>
                          <td>
                            <Button
                              bsStyle="primary"
                              disabled={isPending}
                              onClick={() => this.handleChange(ip, status)}
                            >
                              {isPending ? 'changing' : statusString}
                            </Button>
                          </td>
                          <td><a href={`http://${ip}/update`}>upgrade</a></td>
                        </tr>);
                    })}


          </tbody>
        </Table>
      );
    }
}
