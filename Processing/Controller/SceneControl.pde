HashMap<String, ArrayList<int[]>> lstScenes = new HashMap<String, ArrayList<int[]>>();  //holds all scenes

void FillSceneList()
{
  ((ScrollableList)ctrlP5.getController("scenes"))
    .clear()
    .addItems(lstScenes.keySet().toArray(new String[lstScenes.keySet().size()]));
}

void mouseClicked(MouseEvent m) {
  if (mouseEvent.getClickCount() == 2 && ctrlP5.get("scenes").isMouseOver()) 
  { 
    LoadSceneButton();
    Playback();
  }
}



void LoadSceneButton()
{
  ScrollableList mememe = ctrlP5.get(ScrollableList.class, "scenes");
  String selectedSceneName = (String)(mememe.getItem((int)mememe.getValue()).get("text"));

  lstSamples = (ArrayList<int[]>)lstScenes.get(selectedSceneName).clone();
}

void LoadScene(String sceneName)
{
  println("Loading scene: " +sceneName);

    lstSamples = (ArrayList<int[]>)lstScenes.get(sceneName).clone();
  
}

void DeleteSceneButton()
{  
  ScrollableList mememe = ctrlP5.get(ScrollableList.class, "scenes");
  String selectedSceneName = (String)(mememe.getItem((int)mememe.getValue()).get("text"));

  lstScenes.remove(selectedSceneName);

  FillSceneList();
}

void SaveScene()
{
  if(lstSamples.size()==0)
  {
    println("No samples recorded");
    return;
  }

  println(messageBoxString);

  lstScenes.put(messageBoxString, (ArrayList)lstSamples.clone());

  for (String key : lstScenes.keySet())
    println(key);


  FillSceneList();
}