import { observable, action } from 'mobx';
import { sendData } from '../utils/restCalls';


export class NeighborsStore {
    @observable devicesStatus = {};

    @observable devicePending = {};

    @action requestChangeState(ip) {
      this.devicePending[ip] = true;
      sendData(`http://${ip}/toggle?origin=${window.location.origin}`, 'POST', JSON.stringify({
        relay: 'off',
        ip,
      })).then(action(({ data }) => {
        const json = JSON.parse(data);
        this.devicesStatus[ip] = { status: json.relay };
      })).catch(
        action(({ data }) => {
          this.error = data;
        }),
      ).finally(action(() => {
        delete this.devicePending[ip];
      }));
    }
}

export default new NeighborsStore();
