// Copyright Epic Games, Inc. All Rights Reserved.

using System.IO;
using UnrealBuildTool;

public class Lab4 : ModuleRules
{
	public Lab4(ReadOnlyTargetRules Target) : base(Target)
	{
		PrivateDependencyModuleNames.AddRange(new string[] { "VoiceChat" });
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", 
			"HeadMountedDisplay", "UMG", "XmlParser", "OnlineSubsystemEOS", "OnlineSubsystem", "EngineSettings", "EOSSDK", "WebSockets", "Sockets", "Networking", "Json", "JsonUtilities" });
		
		// Uncomment if you are using online features
		PrivateDependencyModuleNames.Add("OnlineSubsystem");

		//string OpenSSLPath = Path.Combine(ModuleDirectory, "..", "..", "ThirdParty", "OpenSSL");
		//PublicIncludePaths.Add(Path.Combine(OpenSSLPath, "include"));

		//if (Target.Platform == UnrealTargetPlatform.Win64)
		//{
		//	if (Target.Configuration == UnrealTargetConfiguration.Debug)
		//	{
		//		PublicAdditionalLibraries.Add(Path.Combine(OpenSSLPath, "lib/VC/x64/MDd", "libssl.lib"));
		//		PublicAdditionalLibraries.Add(Path.Combine(OpenSSLPath, "lib/VC/x64/MDd", "libcrypto.lib"));
		//	}
		//	else
		//	{
		//		PublicAdditionalLibraries.Add(Path.Combine(OpenSSLPath, "lib/VC/x64/MD", "libssl.lib"));
		//		PublicAdditionalLibraries.Add(Path.Combine(OpenSSLPath, "lib/VC/x64/MD", "libcrypto.lib"));
		//	}
		//}
	}
}
