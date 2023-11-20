using System;
using System.IO;
using System.Linq;
using System.Text;

namespace PooledStrings
{
    internal class Program
    {
        static void Main(string[] args)
        {
            if (args.Length == 2)
            {
                string inFilePath = args[0];
                string outFilePath = args[1];
                Console.WriteLine("Input file: {0}", inFilePath);
                Console.WriteLine("Output file: {0}", outFilePath);

                StringBuilder sb = new StringBuilder();
                sb.AppendLine("#include \"pch.h\"");
                sb.AppendLine("#include <Toshi/Strings/TPString8.h>");
                sb.AppendLine("");
                sb.AppendLine("TOSHI_NAMESPACE_USING");
                sb.AppendLine("");
                sb.AppendLine("// Definitions of global pooled strings");

                string[] strings = File.ReadAllLines(inFilePath);
                uint numStrings = 0;

                for (int i = 0; i < strings.Length; i++)
                {
                    string str = strings[i];

                    if (str.Length > 1 && str.IndexOf(" ") == -1)
                    {
                        sb.AppendFormat("TPString8 g_str_{0};\n", str);
                        numStrings++;
                    }
                    else
                    {
                        Console.WriteLine("Skipping \"{}\" string!", str);
                    }
                }

                sb.AppendLine("");
                sb.AppendLine("static TPString8Initialiser::StringMap oInitStrMap[] = ");
                sb.AppendLine("{");

                for (int i = 0; i < strings.Length; i++)
                {
                    string str = strings[i];

                    if (str.Length > 1 && str.IndexOf(" ") == -1)
                    {
                        sb.AppendFormat("\t{{ &g_str_{0},{1}\"{0}\" }},\n", str, String.Concat(Enumerable.Repeat(" ", 33 - str.Length)));
                    }
                }

                sb.AppendLine("};");
                sb.AppendLine("");
                sb.AppendFormat("static TPString8Initialiser oInitStrNode(oInitStrMap, {0}, TTRUE);\n", numStrings);
                sb.AppendLine("");

                File.WriteAllText(outFilePath, sb.ToString());
                Console.WriteLine("\"{0}\" file has been successfully created.", outFilePath);
            }
        }
    }
}
