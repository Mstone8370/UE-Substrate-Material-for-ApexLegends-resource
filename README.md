![sample](https://github.com/Mstone8370/UE-Substrate-Material-for-ApexLegends-resource/assets/43489974/e134bd7b-f15d-433d-bec3-ca2ca31217d1)

[Sample gif (7MB)](https://github.com/Mstone8370/UE-Substrate-Material-for-ApexLegends-resource/assets/43489974/1aa2e129-bd1f-4f88-92b8-50cace6838a2)

# Unreal Engine Substrate Material for Apex Legends resource

A repository for an Unreal Engine plugin containing Substrate Materials for extracted Apex Legends resources and an auto texture mapping tool.

(Also includes a Post Process Sharpening Material.)

# ***Important Notice***

This project and plugin use ***Substrate***, so you must enable Substrate.
Substrate is an experimental feature that completely replaces the existing Material system but does not guarantee compatibility.
Therefore, it is recommended not to use this plugin in ongoing projects.
If you really want to use it in an ongoing project, duplicate the project first.
Additionally, enable Substrate before adding this plugin.

Substrate can be enabled in the ```Project Settings``` under **Engine->Rendering->Substrate**.

![ss](https://github.com/Mstone8370/UE-Substrate-Material-for-ApexLegends-resource/assets/43489974/1a6206c9-3d8f-4453-8745-579a94baf70a)

Note that Substrate is currently incompatible with hardware ray tracing.
If hardware ray tracing is enabled, rendering issues may occur.
Therefore, you should use Lumen for now.
Substrate is expected to support hardware ray tracing through Lumen in the future.

# About this repository's UE project

This project file includes my PostProcessVolume setup and the plugin code.

You can access both the plugin's contents and my PostProcessVolume setup by cloning this repository.

If you only want the plugin, it's recommended to download the zipped plugin file from the [release page](https://github.com/Mstone8370/UE-Substrate-Material-for-ApexLegends-resource/releases).

This project's Unreal Engine version is ```5.4```

# Installation

## By clonning this repository:

  **Apex Legends Material** plugin is disabled by default, so you should enable it.

![plugin](https://github.com/Mstone8370/UE-Substrate-Material-for-ApexLegends-resource/assets/43489974/a888f091-c736-47b0-a2ea-1e09f71d250a)

## By downloading the plugin from release page:

  Extract the zip file and place the **ApexLegendsMaterial** folder inside your project's ```Plugins``` folder.

  If the ```Plugins``` folder doesn't exist, create a new one.

  After placing the **ApexLegendsMaterial** folder in your project's ```Plugins``` folder, make sure to enable the plugin.

  - As mentioned above, this plugin uses ***Substrate***, which is an experimental feature that completely replaces the existing Material system but does not guarantee compatibility.

    Please be cautious before enabling this plugin, and ensure that Substrate is enabled in the project settings before activating the plugin.

# Usage

* Material

  The Master Material is located in ```Plugins/ApexLegendsMaterial/Materials```.

  In general, use ```M_Master```, and for translucent materials, use ```M_Master_Translucent```.

  It is recommended to create instances of these materials and override the settings.

  - For those not using the Auto Texture Mapping tool: Make sure to uncheck sRGB for the AO, cavity, gloss, and alpha mask textures in texture setting. And set the normal map's ```Texture Group``` to ```WorldNormalMap``` and ```Compression Settings``` to ```Normal map```.

![mat_select](https://github.com/Mstone8370/UE-Substrate-Material-for-ApexLegends-resource/assets/43489974/3c4a90af-acc0-468a-80f4-75fc126aeee8)

---

* Auto Texture Mapping tool

1. In the Skeletal Mesh's **Asset Details** pannel, disconnect all Materials if any are connected by default.

   If any materials are connected, this tool will try to change only textures of that material.

   And if that material is not a Master Material from this plugin, the texture settings will be fail.

   If no Materials are connected to this Skeletal Mesh, this tool will create new instances of Master Material, and textures will be connected to those material instances.

![slotname](https://github.com/Mstone8370/UE-Material-for-ApexLegends-Asset/assets/43489974/e26d26cc-2c46-4575-8382-9e0c07ed1aa3)

2. Create a folder in directory where Skeletal Mesh exists, and import all textures to that folder.

   This tool will search for a folder named ```Textures``` by default, but you can designate another folder name.

![texturefoldername](https://github.com/Mstone8370/UE-Material-for-ApexLegends-Asset/assets/43489974/d52d34d1-3d51-4cb2-9445-a9e3b1aa81b5)

3. **Save all** assets before running the Auto Texture Mapping tool.

4. Right-click the Skeletal Mesh, and select ```Auto Texture Mapping``` inside the **Scripted Asset Actions**.

![right-click](https://github.com/Mstone8370/UE-Material-for-ApexLegends-Asset/assets/43489974/23e92291-fe78-489c-83ee-39d6d0e8d021)

![demo_gif](https://github.com/Mstone8370/UE-Material-for-ApexLegends-Asset/assets/43489974/c9824a30-231f-4f86-96c0-5ad237994dca)

---

* Auto Texture Mapping tool with **Recolor Skins**

  Before running the tool, open the Skeletal Mesh and change the ```Slot Name```s of the materials to match the names of recolor skins.

  For example, if the ```Slot Name``` is ```wraith_lgnd_v19_voidwalker_body```, recolor skin's names will be ```wraith_[recolor skin name]_[parts]``` like ```wraith_lgnd_v19_voidwalker_rc01_gear``` and ```wraith_lgnd_v19_voidwalker_rt01_helmet```.

![recolor](https://github.com/Mstone8370/UE-Material-for-ApexLegends-Asset/assets/43489974/96de7184-26bd-40c5-8daa-a06da53c5595)

---

* The settings for the Auto Texture Mapping tool can be changed in the ```BP_AutoTextureMapping``` Blueprint located in ```Plugins/ApexLegendsMaterial/Util```.

![ATM_settings](https://github.com/Mstone8370/UE-Substrate-Material-for-ApexLegends-resource/assets/43489974/c543f9ba-ffcb-4e26-ba00-ac001c0b4409)

---

* You can watch the demo of my workflow.

  [Auto Texture Mapping tool with recolor skins](https://youtu.be/RVcoh0FYdR4)

  [Unreal Engine Sequencer work with animations](https://youtu.be/UpkA9dgYGuA)

# Issues

## Material issues

* Pixelated Artifacts

  If you find pixelated artifacts, the cause might be the alpha channel of the albedo texture.
  
  By default, the alpha channel of the albedo texture is used as an opacity mask, but there can be issues with some alpha channels.

  Therefore, try unchecking the ```AlbedoAlphaAsOpacityMask``` setting in the Material Instance.

  - Artifact example

![opa_prev](https://github.com/Mstone8370/UE-Substrate-Material-for-ApexLegends-resource/assets/43489974/a55aa2c3-1ff2-4310-b020-999993b83108)

  Alpha channel
  
![alphachannel](https://github.com/Mstone8370/UE-Material-for-ApexLegends-Asset/assets/43489974/c1f01ef5-938a-4f02-83af-d122c2397618)

  Material Instance setting

![matsetting](https://github.com/Mstone8370/UE-Material-for-ApexLegends-Asset/assets/43489974/216d54a0-b167-43f4-99a5-7ddeb43141ad)

  Result

![opa_aft](https://github.com/Mstone8370/UE-Substrate-Material-for-ApexLegends-resource/assets/43489974/3b94f7a9-a4d5-49ed-a4f1-63431c71c9ec)

* Hair Color

  If the hair color appears as too dark, turn off ```CavityAffectAlbedo``` checkbox or lower the ```CavityPow``` value in the Material Instance.

  - Hair example

![cav_prev](https://github.com/Mstone8370/UE-Substrate-Material-for-ApexLegends-resource/assets/43489974/b89f178f-0919-45b5-a988-4c2dc5c7f33f)

![cavity](https://github.com/Mstone8370/UE-Material-for-ApexLegends-Asset/assets/43489974/cb2b18f5-74a9-4ec8-b833-97b3b2865d42)

  or

![cavity_val](https://github.com/Mstone8370/UE-Material-for-ApexLegends-Asset/assets/43489974/bcae8eeb-f033-4854-a12d-2dab2dfd12c2)

  Result

![cav_aft](https://github.com/Mstone8370/UE-Substrate-Material-for-ApexLegends-resource/assets/43489974/3ef7abac-87e0-4c11-8790-d50dec59a91a)

* Translucency Sorting Issue

  When a single mesh has multiple translucent materials, priority sorting may not work correctly, causing the translucent material in the back to cover the one in the front.

  In this case, enable the ```Enable Order Independent Transparency (Experimental)``` option in ```Project Settings``` under **Engine->Rendering->Translucency**.

![translucency](https://github.com/Mstone8370/UE-Substrate-Material-for-ApexLegends-resource/assets/43489974/9b71466d-fb79-41dc-ad16-056e3c63d8f7)

  In the image below, the left side shows an example with sorting issues, and the right side shows the issue resolved using this option.

![translucent_cmp](https://github.com/Mstone8370/UE-Substrate-Material-for-ApexLegends-resource/assets/43489974/96ec8971-6cd0-4a15-aaf7-70589c662b01)

## Auto Texture Mapping tool issue

  If the Auto Texture Mapping tool is not working and error logs are printed in the ```Output Log```, the reason might be an old reference issue.

  Try deleting all Material Instances connected to the Skeletal Mesh, and in the content browser, right-click the current folder and select ```Update Redirector References``` to fix the old references.

## Movie Render Queue issue

  When using the Movie Render Queue, there could be an issue where shaders are compiled for every frame.

  This issue can be caused by materials that fail to compile, and Substrate can be the cause of this issue.

  If you find logs in the ```[project path]/Saved/Logs``` folder indicating that material compilation has failed, you will need to fix those materials.

  If those problematic materials belong to an unused plugin, simply disabling that plugin may resolve the issue.

# Experimental Feature

The ```M_Master``` material has an **Anisotropy** option.

Some resources have all the necessary information for anisotropy, resulting in much better outcomes when applied, but in some cases, the necessary information is not enough.

Anisotropy enhances detail, but its effect is subtle, so it is disabled by default.

However, if you want to use it, you can enable it in the **Material Instance** settings, and in some cases, you might need to directly modify the ```M_Master``` Material.

The following images compare the results enabled and disabled the **Anisotropy** option.

Please note that these examples show the most pronounced effect.

The image on the left is without anisotropy (isotropic), right is anisotropy.

![aniso_comp](https://github.com/Mstone8370/UE-Substrate-Material-for-ApexLegends-resource/assets/43489974/534dc91d-40d3-4a31-b1ea-cb9ab27270fa)

