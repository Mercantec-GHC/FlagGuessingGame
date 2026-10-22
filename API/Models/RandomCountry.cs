namespace ArduinoLandeQuizAPI.Models
{
    public class RandomCountry
    {
        public string FlagCode { get; set; }

        public List<string> Choices { get; set; }

        public int CorrectChoiceIndex { get; set; }
    }
}
