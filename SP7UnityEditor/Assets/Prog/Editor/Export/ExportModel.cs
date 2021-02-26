﻿using System.Collections;
using System.Collections.Generic;
using UnityEditor;
using UnityEngine;
using UnityEngine.SceneManagement;


[System.Serializable]
class ModelLink
{
    public int instanceID;
    public int assetID;
}

[System.Serializable]
class ModelCollection
{
    public List<ModelLink> modelLinks;
}

public class ExportModel
{
    public static void Export(Scene aScene)
    {
        ModelCollection fbxLinks = new ModelCollection();
        fbxLinks.modelLinks = new List<ModelLink>();

        Renderer[] allrenderers = GameObject.FindObjectsOfType<Renderer>();

        foreach(Renderer renderer in allrenderers)
        {
            if (renderer.GetComponent<PolybrushFBX>() != null)
                continue;

            if(Json.TryIsValidExport(renderer, out GameObject prefabParent))
            {
                if (renderer.TryGetComponent(out MeshFilter meshFilter))
                {
                    GameObject modelAsset = PrefabUtility.GetCorrespondingObjectFromOriginalSource(meshFilter).gameObject;
                    if(modelAsset.transform.parent != null)
                    {
                        modelAsset = modelAsset.transform.parent.gameObject;
                    }
                    

                    if (modelAsset == null)
                        continue;

                    ModelLink link = new ModelLink();
                    link.assetID = modelAsset.transform.GetInstanceID();
                    link.instanceID = prefabParent.transform.GetInstanceID();
                    fbxLinks.modelLinks.Add(link);
                }
            }           
        }
        Json.ExportToJson(fbxLinks, aScene.name);
    }
}
