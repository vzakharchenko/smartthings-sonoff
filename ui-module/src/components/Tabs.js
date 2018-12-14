import React from 'react';
import { inject, observer } from 'mobx-react';
import Tabs from 'react-bootstrap/lib/Tabs';
import Tab from 'react-bootstrap/lib/Tab';
import TabContent from 'react-bootstrap/lib/TabContent';
import InfoTab from './InfoTab';
import ConfigurationTab from './confTab';
import SmartThingsTab from './SmartThingsTab';


export default
@inject('tabsStore')
@observer
class DeviceTabs extends React.Component {
    handleChange = (event, value) => {
      this.props.tabsStore.setTab(value);
    };

    selectInfo() {
      this.props.tabsStore.setTab('Info');
    }

    selectConfiguration() {
      this.props.tabsStore.setTab('Configuration');
    }

    render() {
      const { tabId } = this.props.tabsStore;

      return (
        <div>
          <Tabs defaultActiveKey={tabId} id="uncontrolled-tab-example">
            <Tab eventKey="Info" title="Info">
              <TabContent>
                <InfoTab />
              </TabContent>
            </Tab>
            <Tab eventKey="SmartThingsTab" title="SmartThings">
              <TabContent>
                <SmartThingsTab />
              </TabContent>
            </Tab>
            <Tab eventKey="ConfigurationTab" title="Configuration">
              <TabContent>
                <ConfigurationTab />
              </TabContent>
            </Tab>
          </Tabs>
        </div>
      );
    }
}
