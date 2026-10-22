using System.Text.Json.Serialization;

namespace FlagGuessingGameAPI.Models
{
    public class Country
    {
        public CountryName Name { get; set; }

        public CountryFlags Flags { get; set; }

        [JsonPropertyName("cca3")]
        public string CCA3 { get; set; }
    }

    public class CountryName
    {
        public string Common { get; set; }
    }

    public class CountryFlags
    {
        public string Png { get; set; }
    }
}
