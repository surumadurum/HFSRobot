HashMap<String,ArrayList<int[]>> lstScenes = new HashMap<String,ArrayList<int[]>>();  //holds all scenes

void FillSceneList()
{
  
  
  ScrollableList mememe;
  mememe=(ScrollableList)ctrlP5.getController("scenes");
//  for(String scene_name : lstScenes.keySet())
  mememe.clear();
    mememe.addItems(lstScenes.keySet().toArray(new String[lstScenes.keySet().size()]));
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
  ScrollableList mememe = ctrlP5.get(ScrollableList.class,"scenes");
  String selectedSceneName = (String)(mememe.getItem((int)mememe.getValue()).get("text"));
  
  lstSamples = (ArrayList<int[]>)lstScenes.get(selectedSceneName).clone();
}

void DeleteSceneButton()
{  ScrollableList mememe = ctrlP5.get(ScrollableList.class,"scenes");
   String selectedSceneName = (String)(mememe.getItem((int)mememe.getValue()).get("text"));
   
   lstScenes.remove(selectedSceneName);
   
   FillSceneList();
}

void SaveScene()
{

    println(messageBoxString);
    
    lstScenes.put(messageBoxString,(ArrayList)lstSamples.clone());
    
    for(String key : lstScenes.keySet())
      println(key);
    
    
    FillSceneList();  
  
}