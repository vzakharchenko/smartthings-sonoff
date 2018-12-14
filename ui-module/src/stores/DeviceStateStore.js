import { observable, action } from 'mobx';
import { fetchData, sendData } from '../utils/restCalls';

const serverUrl = process.env.SERVER_URL;


export class DeviceStateStore {
    @observable
    relay = false;

    @observable uptime = 0;

    @observable ssid = '';

    @observable ip = '';

    @observable mac = '';

    @observable applicationId = '';

    @observable accessToken = '';

    @observable smartThingsUrl = '';

    @observable smartthingsName = '';

    @observable smartthingsStatus = '';

    @observable versionFirmware = '';

    @observable pow = false;

    @observable defaultState = 0;

    @observable isLoading = false;

    @observable isChanging = false;

    @observable isSaving = false;

    @observable isValid = false;


    @observable error = undefined;

    validationForm() {
      let isValid = true;
      if (!this.applicationId
          || this.applicationId.length === 0
          || this.applicationId.length > 127) {
        isValid = false;
      }
      if (!this.accessToken
          || this.accessToken.length === 0
          || this.accessToken.length > 127) {
        isValid = false;
      }
      if (!this.smartThingsUrl
          || this.smartThingsUrl.length === 0
          || this.smartThingsUrl.length > 127) {
        isValid = false;
      }
      if (this.defaultState < 0
          || this.defaultState > 3) {
        isValid = false;
      }
      this.isValid = isValid;
    }

    parseState(data) {
      const res = JSON.parse(data);
      this.relay = !!(res.relay && res.relay === 'on');
      this.uptime = res.uptime;
      this.ssid = res.ssid;
      this.ip = res.ip;
      this.mac = res.mac;
      this.applicationId = res.applicationId;
      this.accessToken = res.accessToken;
      this.smartThingsUrl = res.smartThingsUrl;
      this.smartthingsName = res.smartthingsName;
      this.smartthingsStatus = res.smartthingsStatus;
      this.pow = res.pow;
      this.defaultState = res.defaultState;
      this.versionFirmware = res.versionFirmware;
      this.validationForm();
    }


    @action load() {
      this.isLoading = true;
      fetchData(`${serverUrl}state`).then(action(({ data }) => {
        this.parseState(data);
      })).catch(
        action(({ data }) => {
          this.error = data;
        }),
      ).finally(action(() => {
        this.isLoading = false;
      }));
    }

    @action toggleRelay() {
      this.isChanging = true;
      const url = `${serverUrl}toggle`;
      sendData(url, 'POST', JSON.stringify({
        relay: this.relay ? 'on' : 'off',
        ip: '192.100.200.13',
        mac: '80:7D:3A:45:E8:16',
      })).then(action(({ data }) => {
        this.parseState(data);
      })).catch(
        action(({ error }) => {
          this.error = error.data;
        }),
      ).finally(action(() => {
        this.isChanging = false;
      }));
    }

    @action setFormData(elementName, elementValue) {
      this[elementName] = elementValue;
      this.validationForm();
    }

    @action saveForm() {
      this.isSaving = true;
      const postData = `applicationId=${encodeURIComponent(this.applicationId)}&accessToken=${encodeURIComponent(this.accessToken)}&smartThingsUrl=${encodeURIComponent(this.smartThingsUrl)}&defaultState=${this.defaultState}`;
      sendData(`${serverUrl}config`, 'POST', postData, {
        Accept: 'text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,image/apng,application/json,*/*;q=0.8',
        'Content-Type': 'application/x-www-form-urlencoded',
      }).then(action(({ data }) => {
        this.parseState(data);
      })).catch(
        action(({ data }) => {
          this.isSaving = false;
          this.error = data;
        }),
      ).finally(action(() => {
        this.isSaving = false;
      }));
    }
}

export default new DeviceStateStore();
