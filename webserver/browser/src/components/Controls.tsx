import { ButtonPanel } from './ButtonPanel.tsx';
import { Input } from './Input.tsx';
import { Joystick } from './Joystick';

export function Controls() {
  return(
    <div>
      <Input />
      <ButtonPanel />
      <Joystick />
    </div>
  )
}
