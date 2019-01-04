import React from 'react';
import { inject, observer } from 'mobx-react';
import FormGroup from 'react-bootstrap/lib/FormGroup';
import ControlLabel from 'react-bootstrap/lib/ControlLabel';
import FormControl from 'react-bootstrap/lib/FormControl';
import HelpBlock from 'react-bootstrap/lib/HelpBlock';
import Button from 'react-bootstrap/lib/Button';
import Image from 'react-bootstrap/lib/Image';


const staticServerUrl = process.env.STATIC_SERVER_URL;

export default
@inject('deviceStateStore')
@observer
class ConfigurationTab extends React.Component {
  getValidationUrlState(parameterName) {
    const parameter = this.props.deviceStateStore[parameterName];
    return parameter.length > 128 || parameter.length === 0 ? 'error' : null;
  }

    handleSave = () => {
      this.props.deviceStateStore.saveForm();
    }

    handleChangeState = (event) => {
      this.props.deviceStateStore.setFormData(event.target.name, event.target.value);
    }

    render() {
      const {
        smartThingsUrl,
        applicationId,
        accessToken,
        defaultState,
        isSaving,
        isValid,
        deviceType,
        openTimeOut,
      } = this.props.deviceStateStore;
      return (
        <form>
          <FormGroup
            controlId="smartThingsUrl"
            validationState={this.getValidationUrlState('smartThingsUrl')}
          >
            <ControlLabel>SmartThings Url</ControlLabel>
            <FormControl
              type="text"
              name="smartThingsUrl"
              value={smartThingsUrl}
              placeholder="Enter Smartthings Url"
              onChange={this.handleChangeState}
            />
            <FormControl.Feedback />
            <HelpBlock>
              <Image style={{ maxWidth: '80%', height: 'auto' }} src={`${staticServerUrl}SmartThingsUrl.png`} />
            </HelpBlock>
          </FormGroup>
          <FormGroup
            controlId="applicationId"
            validationState={this.getValidationUrlState('applicationId')}
          >
            <ControlLabel>SmartThings Application Id</ControlLabel>
            <FormControl
              type="text"
              name="applicationId"
              value={applicationId}
              placeholder="Enter application Id"
              onChange={this.handleChangeState}
            />
            <FormControl.Feedback />
            <HelpBlock>
              <Image style={{ maxWidth: '80%', height: 'auto' }} src={`${staticServerUrl}ApplicationId.png`} />
            </HelpBlock>
          </FormGroup>
          <FormGroup
            controlId="accessToken"
            validationState={this.getValidationUrlState('accessToken')}
          >
            <ControlLabel>SmartThings AccessToken</ControlLabel>
            <FormControl
              type="text"
              name="accessToken"
              value={accessToken}
              placeholder="Enter application Id"
              onChange={this.handleChangeState}
            />
            <FormControl.Feedback />
            <HelpBlock>
              {' '}
              <HelpBlock>
                <Image style={{ maxWidth: '80%', height: 'auto' }} src={`${staticServerUrl}accessToken.png`} />
              </HelpBlock>
            </HelpBlock>
          </FormGroup>
          <FormGroup
            controlId="deviceType"
          >
            <ControlLabel>Device Type</ControlLabel>
            <FormControl
              componentClass="select"
              placeholder="Device Type"
              name="deviceType"
              defaultValue={deviceType}
              onChange={this.handleChangeState}
            >
              <option value="0">Relay Sonoff</option>
              <option value="1">Vizit Intercom</option>
            </FormControl>
            <FormControl.Feedback />
          </FormGroup>
          {parseInt(deviceType, 10) !== 1 ? (
            <FormGroup
              controlId="defaultState"
            >
              <ControlLabel>PowerState at startup</ControlLabel>
              <FormControl
                componentClass="select"
                placeholder="PowerState at startup"
                name="defaultState"
                defaultValue={defaultState}
                onChange={this.handleChangeState}
              >
                <option value="0">Off</option>
                <option value="1">On</option>
                <option value="2">Latest</option>
                <option value="3">SmartThings</option>
              </FormControl>
              <FormControl.Feedback />
            </FormGroup>
          ) : null}
          {parseInt(deviceType, 10) === 1 ? (
            <FormGroup
              controlId="openTimeOut"
            >
              <ControlLabel>SmartThings Open Door TimeOut</ControlLabel>
              <FormControl
                type="number"
                name="openTimeOut"
                value={openTimeOut}
                placeholder="open Door TimeOut"
                onChange={this.handleChangeState}
              />
              <FormControl.Feedback />
            </FormGroup>
          ) : null
            }

          <Button
            bsStyle="primary"
            disabled={isSaving || !isValid}
            onClick={!isSaving ? this.handleSave : null}
          >
            {isSaving ? 'Saving...' : 'Save'}
          </Button>
        </form>
      );
    }
}
