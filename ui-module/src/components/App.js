import React from 'react';
import { inject, observer } from 'mobx-react';
import Panel from 'react-bootstrap/lib/Panel';
import Image from 'react-bootstrap/lib/Image';
import Loading from './Loading';
import DeviceTabs from './Tabs';

const staticServerUrl = process.env.STATIC_SERVER_URL;

export default
@inject('deviceStateStore')
@observer
class App extends React.Component {
  componentDidMount() {
    if (!this.props.deviceStateStore.isLoading) {
      this.props.deviceStateStore.load();
    }
  }

  render() {
    const { isLoading } = this.props.deviceStateStore;
    return (
      <Panel>
        <Panel.Heading>

          <Image
            src={`${staticServerUrl}logo.png`}
            style={{
              maxWidth: '20%',
              height: 'auto',
              display: 'block',
              marginLeft: 'auto',
              marginRight: 'auto',
              width: '50%',
            }}
          />

        </Panel.Heading>
        <Panel.Body style={{
          height: 'auto',
          display: 'block',
          marginLeft: 'auto',
          marginRight: 'auto',
          width: '50%',
        }}
        >
          {isLoading ? <Loading /> : <DeviceTabs /> }

        </Panel.Body>
      </Panel>
    );
  }
}
