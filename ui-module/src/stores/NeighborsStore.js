import { observable, action } from 'mobx';
import { fetchData } from '../utils/restCalls';


export class NeighborsStore {
    @observable devicesStatus = {};

    @observable devicePending = {};

    @action requestChangeState(ip, state) {
      this.devicePending[ip] = true;
      fetchData(`http://${ip}/${state}`).then(action(({ data }) => {
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
