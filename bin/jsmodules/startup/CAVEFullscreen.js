var ic = input.RegisterInputContextRaw("CAVEStereoFullscreenToggler", 100);
ic.KeyPressed.connect(OnKeyPressed)
function OnKeyPressed(keyEvent)
{
   if (keyEvent.text == "L")
      framework.GetModuleByName("CAVEStereo").Manager().ShowFullscreen();
}


