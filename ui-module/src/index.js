import React from 'react';
import ReactDOM from 'react-dom';
import { Provider } from 'mobx-react';
import deviceStateStore from './stores/DeviceStateStore';
import tabsStore from './stores/TabsStore';
import App from './components/App';

const stores = {
  deviceStateStore,
  tabsStore,
};

// For easier debugging
window._____APP_STATE_____ = stores; // eslint-disable-line no-underscore-dangle


const Index = () => (
  <Provider {...stores}>
    <App />
  </Provider>
);

ReactDOM.render(<Index />, document.getElementById('index'));
